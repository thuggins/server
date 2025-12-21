/**
@file worker.c
@brief Per-connection handler thread for HTTP/WebSocket.

Provides:
- Reads first request per connection
- WebSocket echo loop (text, ping/pong)
- Static file serving with connection-close semantics
@note Closes client socket when done.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "ws.h"
#include "http.h"
#include "worker.h"

/**
@brief Thread proc for a single client connection (HTTP/WebSocket).
@note Closes the client socket before returning.
*/
unsigned __stdcall client_worker(void* arg) {
    client_ctx_t* ctx = (client_ctx_t*)arg;
    SOCKET client = ctx->client;
    struct sockaddr_in clientAddr = ctx->addr;
    free(ctx);

    char clientIP[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN)) {
        strcpy(clientIP, "unknown");
    }

    // Only SSL connections are supported
    SSL* ssl = SSL_new((SSL_CTX*)ctx->ssl_ctx);
    if (!ssl) {
        closesocket(client);
        return 0;
    }
    SSL_set_fd(ssl, (int)client);
    if (SSL_accept(ssl) <= 0) {
        SSL_free(ssl);
        closesocket(client);
        return 0;
    }
    char request[2048] = {0};
    int recvBytes = SSL_read(ssl, request, sizeof(request) - 1);
    if (recvBytes <= 0) {
        SSL_free(ssl);
        closesocket(client);
        return 0;
    }

    // Extract request line (for logging)
    char requestLine[256] = {0};
    char* line_end = strstr(request, "\r\n");
    if (line_end) {
        int len = (int)(line_end - request);
        if (len > 255)
            len = 255;
        strncpy(requestLine, request, len);
    }

    // WebSocket upgrade vs static file
    if (strstr(request, "Upgrade: websocket") != NULL ||
        strstr(request, "upgrade: websocket") != NULL) {
        if (websocket_handshake_ssl(ssl, request)) {
            // Send Home HTML page on initial connection
            websocket_send_text_ssl(ssl, "<h2>Home</h2><p>Welcome to the Home page!</p>");
            char msg[1024];
            int mlen;
            while ((mlen = websocket_read_text_ssl(ssl, msg, sizeof(msg) - 1)) > 0 || mlen == -2) {
                if (mlen == -2)
                    continue; // control frame handled

                // Try to parse as JSON for page requests
                if (msg[0] == '{') {
                    char page[32] = {0};
                    const char* ptype = strstr(msg, "\"type\"");
                    const char* ppage = strstr(msg, "\"page\"");
                    if (ptype && strstr(ptype, "page")) {
                        if (ppage) {
                            const char* colon = strchr(ppage, ':');
                            if (colon) {
                                const char* quote1 = strchr(colon, '"');
                                if (quote1) {
                                    const char* quote2 = strchr(quote1 + 1, '"');
                                    if (quote2 && quote2 - quote1 - 1 < (int)sizeof(page)) {
                                        strncpy(page, quote1 + 1, quote2 - quote1 - 1);
                                        page[quote2 - quote1 - 1] = 0;
                                    }
                                }
                            }
                        }
                        // Send HTML for the requested page
                        const char* html = NULL;
                        if (strcmp(page, "home") == 0) {
                            html = "<h2>Home</h2><p>Welcome to the Home page!</p>";
                        } else if (strcmp(page, "about") == 0) {
                            html = "<h2>About</h2><p>This is a demo WebSocket-driven site. Powered "
                                   "by C!</p>";
                        } else if (strcmp(page, "contact") == 0) {
                            html = "<h2>Contact</h2><p>Contact us at <a "
                                   "href='mailto:demo@example.com'>demo@example.com</a></p>";
                        } else {
                            html = "<h2>Not Found</h2><p>Page not found.</p>";
                        }
                        websocket_send_text_ssl(ssl, html);
                        continue;
                    }
                }
                // Fallback: ignore or send a default page
                // websocket_send_text_ssl(ssl, "<h2>Unknown request</h2>");
            }
        }
        SSL_free(ssl);
        closesocket(client);
    } else {
        // Serve static file based on the requested path
        char method[8] = {0};
        char path[256] = {0};
        sscanf(request, "%7s %255s", method, path);
        http_serve_file_ssl(ssl, path);
        SSL_free(ssl);
        closesocket(client);
    }
    return 0;
}

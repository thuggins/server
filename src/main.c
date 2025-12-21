
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ws.h"
#include "http.h"
#include "worker.h"
#include "ssl_helper.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

// Entry point: initialize Winsock, OpenSSL, and start accept loop on port 8443 (HTTPS/WSS only)
int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    // Create SSL_CTX for HTTPS/WSS
    SSL_CTX* ssl_ctx = create_ssl_ctx("localhost-cert.pem", "localhost-key.pem");
    if (!ssl_ctx) {
        printf("Failed to create SSL_CTX.\n");
        WSACleanup();
        return 1;
    }

    // Create TCP socket for HTTPS/WSS only
    SOCKET sock_ssl = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ssl == INVALID_SOCKET) {
        printf("SSL socket creation failed\n");
        SSL_CTX_free(ssl_ctx);
        WSACleanup();
        return 1;
    }
    int reuse = 1;
    setsockopt(sock_ssl, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
    struct sockaddr_in serverAddrSSL;
    serverAddrSSL.sin_family = AF_INET;
    serverAddrSSL.sin_addr.s_addr = INADDR_ANY;
    serverAddrSSL.sin_port = htons(8443);
    if (bind(sock_ssl, (struct sockaddr*)&serverAddrSSL, sizeof(serverAddrSSL)) == SOCKET_ERROR) {
        printf("SSL Bind failed\n");
        closesocket(sock_ssl);
        SSL_CTX_free(ssl_ctx);
        WSACleanup();
        return 1;
    }
    listen(sock_ssl, 16);
    printf("Server running on https://localhost:8443\n");
    printf("Press Ctrl+C to stop.\n");

    // Accept loop: spawn worker per connection (HTTPS/WSS only)
    while (1) {
        struct sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET client = accept(sock_ssl, (struct sockaddr*)&clientAddr, &clientLen);
        if (client != INVALID_SOCKET) {
            char ipbuf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, ipbuf, sizeof(ipbuf));
            client_ctx_t* ctx = (client_ctx_t*)malloc(sizeof(client_ctx_t));
            if (!ctx) {
                closesocket(client);
                continue;
            }
            ctx->client = client;
            ctx->addr = clientAddr;
            ctx->ssl_ctx = ssl_ctx;
            uintptr_t th = _beginthreadex(NULL, 0, client_worker, ctx, 0, NULL);
            if (th)
                CloseHandle((HANDLE)th);
            else
                client_worker(ctx);
        }
    }

    closesocket(sock_ssl);
    SSL_CTX_free(ssl_ctx);
    WSACleanup();
    return 0;
}

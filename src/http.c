#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "common.h"
#include <openssl/ssl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "common.h"
#include <openssl/ssl.h>

// Send a minimal HTTP response header (SSL)
static void http_send_header_ssl(SSL* ssl, const char* status_line, const char* content_type,
                                 size_t len) {
    char header[COMMON_HEADER_BUF_SIZE];
    int n = snprintf(header, sizeof(header),
                     "%s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     status_line, content_type, len);
    SSL_write(ssl, header, n);
}

// Send a 200 OK response with the provided body (SSL)
void http_send_response_ssl(void* vssl, const char* content_type, const unsigned char* data,
                            size_t len) {
    SSL* ssl = (SSL*)vssl;
    http_send_header_ssl(ssl, "HTTP/1.1 200 OK", content_type, len);
    SSL_write(ssl, (const char*)data, (int)len);
}

// Send a simple 404 Not Found HTML response (SSL)
void http_send_404_ssl(void* vssl) {
    SSL* ssl = (SSL*)vssl;
    const char* body = "<h1>404 Not Found</h1>";
    http_send_header_ssl(ssl, "HTTP/1.1 404 Not Found", "text/html", strlen(body));
    SSL_write(ssl, body, (int)strlen(body));
}

// Serve a file from ./public by HTTP path (SSL)
void http_serve_file_ssl(void* vssl, const char* path) {
    SSL* ssl = (SSL*)vssl;
    char full[COMMON_MAX_PATH];
    if (strcmp(path, "/") == 0)
        path = "/index.html";
    snprintf(full, sizeof(full), "public%s", path);
    FILE* f = fopen(full, "rb");
    if (!f) {
        http_send_404_ssl(ssl);
        return;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc(len);
    if (!buf) {
        fclose(f);
        http_send_404_ssl(ssl);
        return;
    }
    fread(buf, 1, len, f);
    fclose(f);
    const char* ct = "application/octet-stream";
    const char* ext = strrchr(full, '.');
    if (ext) {
        if (strcmp(ext, ".html") == 0)
            ct = "text/html";
        else if (strcmp(ext, ".js") == 0)
            ct = "application/javascript";
        else if (strcmp(ext, ".css") == 0)
            ct = "text/css";
    }
    http_send_response_ssl(ssl, ct, buf, (size_t)len);
    free(buf);
}

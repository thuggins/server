/**
@file http.c
@brief Minimal HTTP helpers to serve static files from ./public.

Provides:
- 200 OK and 404 helpers
- Simple MIME type selection for .html/.js/.css
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "common.h"
#include <openssl/ssl.h>

/**
@brief Send a minimal HTTP response header.
@param client Connected client socket.
@param status_line Status line (e.g., "HTTP/1.1 200 OK").
@param content_type MIME type for the body.
@param len Body length in bytes.
*/
static void http_send_header(SOCKET client, const char* status_line, const char* content_type,
                             size_t len) {
    char header[COMMON_HEADER_BUF_SIZE];
    int n = snprintf(header, sizeof(header),
                     "%s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     status_line, content_type, len);
    send(client, header, n, 0);
}

static void http_send_header_ssl(SSL* ssl, const char* status_line, const char* content_type,
                                 size_t len) {
    char header[COMMON_HEADER_BUF_SIZE];
    int n = snprintf(header, sizeof(header),
                     "%s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     status_line, content_type, len);
    SSL_write(ssl, header, n);
}

/**
@brief Send a 200 OK response with the provided body.
*/
void http_send_response(SOCKET client, const char* content_type, const unsigned char* data,
                        size_t len) {
    http_send_header(client, "HTTP/1.1 200 OK", content_type, len);
    send(client, (const char*)data, (int)len, 0);
}

void http_send_response_ssl(void* vssl, const char* content_type, const unsigned char* data,
                            size_t len) {
    SSL* ssl = (SSL*)vssl;
    http_send_header_ssl(ssl, "HTTP/1.1 200 OK", content_type, len);
    SSL_write(ssl, (const char*)data, (int)len);
}

/**
@brief Send a simple 404 Not Found HTML response.
*/
void http_send_404(SOCKET client) {
    const char* body = "<h1>404 Not Found</h1>";
    http_send_header(client, "HTTP/1.1 404 Not Found", "text/html", strlen(body));
    send(client, body, (int)strlen(body), 0);
}
void http_send_404_ssl(void* vssl) {
    SSL* ssl = (SSL*)vssl;
    const char* body = "<h1>404 Not Found</h1>";
    http_send_header_ssl(ssl, "HTTP/1.1 404 Not Found", "text/html", strlen(body));
    SSL_write(ssl, body, (int)strlen(body));
}

/**
@brief Serve a file from ./public corresponding to the requested path.
*/
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
void http_serve_file(SOCKET client, const char* path) {
    char full[COMMON_MAX_PATH];
    if (strcmp(path, "/") == 0)
        path = "/index.html";
    snprintf(full, sizeof(full), "public%s", path);
    FILE* f = fopen(full, "rb");
    if (!f) {
        http_send_404(client);
        return;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc(len);
    if (!buf) {
        fclose(f);
        http_send_404(client);
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
    http_send_response(client, ct, buf, (size_t)len);
    free(buf);
}

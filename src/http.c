#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// Send a minimal HTTP response header
static void http_send_header(int client, const char* status_line, const char* content_type,
                             size_t len) {
    char header[COMMON_HEADER_BUF_SIZE];
    int n = snprintf(header, sizeof(header),
                     "%s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     status_line, content_type, len);
    send(client, header, n, 0);
}

// Send a 200 OK response with the provided body
void http_send_response(int client, const char* content_type, const unsigned char* data,
                        size_t len) {
    http_send_header(client, "HTTP/1.1 200 OK", content_type, len);
    send(client, (const char*)data, (int)len, 0);
}

// Send a simple 404 Not Found HTML response
void http_send_404(int client) {
    const char* body = "<h1>404 Not Found</h1>";
    http_send_header(client, "HTTP/1.1 404 Not Found", "text/html", strlen(body));
    send(client, body, (int)strlen(body), 0);
}

// Serve a file from ./public by HTTP path
void http_serve_file(int client, const char* path) {
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

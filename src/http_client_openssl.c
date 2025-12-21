
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Perform a minimal HTTPS GET request using OpenSSL and sockets.
int fetch_https(const char* host, const char* path, char* out, int out_size) {
    int ret = -1;
    int sock = -1;
    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return -1;
    struct hostent* he = gethostbyname(host);
    if (!he) {
        WSACleanup();
        return -1;
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(443);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        goto cleanup;
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        goto cleanup;
    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx)
        goto cleanup;
    ssl = SSL_new(ctx);
    if (!ssl)
        goto cleanup;
    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) <= 0)
        goto cleanup;
    char req[512];
    snprintf(req, sizeof(req),
             "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: C-OpenSSL\r\n\r\n",
             path, host);
    SSL_write(ssl, req, strlen(req));
    int n, total = 0;
    while ((n = SSL_read(ssl, out + total, out_size - total - 1)) > 0) {
        total += n;
        if (total >= out_size - 1)
            break;
    }
    out[total] = 0;
    ret = total;
cleanup:
    if (ssl)
        SSL_free(ssl);
    if (ctx)
        SSL_CTX_free(ctx);
    if (sock != INVALID_SOCKET)
        closesocket(sock);
    WSACleanup();
    return ret;
}

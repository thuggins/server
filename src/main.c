/**
@file main.c
@brief Demo HTTP + WebSocket server entry point (Windows).

Provides:
- Accept loop on TCP port 8080
- Serves static files from ./public
- Upgrades /ws requests to WebSocket and echoes text
Modules: ws (RFC 6455), http (static files), worker (per-connection thread)
*/
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "ws.h"
#include "http.h"
#include "worker.h"

/**
@brief Initialize Winsock, start listening on port 8080,
and dispatch each accepted connection to a worker thread.
@return Process exit code (0 on normal termination).
*/
int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create TCP socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    // Enable address reuse
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    // Bind to 0.0.0.0:8080
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Listen
    listen(sock, 16);
    printf("Server running on http://localhost:8080\n");
    printf("Press Ctrl+C to stop.\n");

    // Accept loop: spawn worker per connection
    while (1) {
        struct sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET client = accept(sock, (struct sockaddr*)&clientAddr, &clientLen);
        if (client == INVALID_SOCKET)
            continue;

        client_ctx_t* ctx = (client_ctx_t*)malloc(sizeof(client_ctx_t));
        if (!ctx) {
            closesocket(client);
            continue;
        }
        ctx->client = client;
        ctx->addr = clientAddr;

        uintptr_t th = _beginthreadex(NULL, 0, client_worker, ctx, 0, NULL);
        if (th) {
            CloseHandle((HANDLE)th);
        } else {
            // If we fail to create a thread, handle synchronously
            client_worker(ctx);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

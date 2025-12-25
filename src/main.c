
#include "http.h"
#include "weather.h"
#include "worker.h"
#include "ws.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_THREADS 1024

// Entry point: initialize Winsock, OpenSSL, and start accept loop on port 8443 (HTTPS/WSS only)
int main() {

    weather_curl_init();

    // Create TCP socket for HTTP only
    int sock_http = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_http < 0) {
        perror("HTTP socket creation failed");
        return 1;
    }
    int reuse = 1;
    setsockopt(sock_http, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
    if (bind(sock_http, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("HTTP Bind failed");
        close(sock_http);
        return 1;
    }
    listen(sock_http, 16);
    printf("Server running on http://localhost:8080\n");
    printf("Press Ctrl+C to stop.\n");

    pthread_t thread_handles[MAX_THREADS];
    int thread_count = 0;
    int running = 1;
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int client = accept(sock_http, (struct sockaddr*)&clientAddr, &clientLen);
        if (client >= 0) {
            char ipbuf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, ipbuf, sizeof(ipbuf));
            client_ctx_t* ctx = (client_ctx_t*)malloc(sizeof(client_ctx_t));
            if (!ctx) {
                close(client);
                continue;
            }
            ctx->client = client;
            ctx->addr = clientAddr;
            if (thread_count < MAX_THREADS) {
                if (pthread_create(&thread_handles[thread_count++], NULL,
                                   (void* (*)(void*))client_worker, ctx) != 0) {
                    // If thread creation fails, handle in main thread
                    client_worker(ctx);
                }
            } else {
                // If too many threads, handle in main thread
                client_worker(ctx);
            }
        }
    }

    close(sock_http);

    // Wait for all worker threads to finish
    for (int i = 0; i < thread_count; ++i) {
        pthread_join(thread_handles[i], NULL);
    }

    weather_curl_cleanup();
    return 0;
}

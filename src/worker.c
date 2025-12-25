#include "worker.h"
#include "http.h"
#include "weather.h"
#include "ws.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Thread proc for a single client connection (HTTP/WebSocket).
void* client_worker(void* arg) {
    client_ctx_t* ctx = (client_ctx_t*)arg;
    int client = ctx->client;
    struct sockaddr_in clientAddr = ctx->addr;

    char clientIP[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN)) {
        strcpy(clientIP, "unknown");
    }

    if (!ctx) {
        fprintf(stderr, "ERROR: ctx is NULL!\n");
        close(client);
        return 0;
    }

    char request[2048] = {0};
    int recvBytes = recv(client, request, sizeof(request) - 1, 0);
    if (recvBytes <= 0) {
        close(client);
        return 0;
    }

    // WebSocket upgrade vs static file
    if (strstr(request, "Upgrade: websocket") != NULL ||
        strstr(request, "upgrade: websocket") != NULL) {
        if (websocket_handshake(client, request)) {
            char msg[1024];
            int mlen;
            int weather_in_progress = 0;
            while ((mlen = websocket_read_text(client, msg, sizeof(msg) - 1)) > 0 || mlen == -2) {
                if (mlen == -2)
                    continue;
                if (strncmp(msg, "{\"type\":\"weather_request\"", 25) == 0 &&
                    !weather_in_progress) {
                    weather_in_progress = 1;
                    // Prepare 10 capitals
                    Location capitals[10] = {{"Montgomery", "AL", 32.3777, -86.3000},
                                             {"Juneau", "AK", 58.3019, -134.4197},
                                             {"Phoenix", "AZ", 33.4484, -112.0740},
                                             {"Little Rock", "AR", 34.7465, -92.2896},
                                             {"Sacramento", "CA", 38.5816, -121.4944},
                                             {"Denver", "CO", 39.7392, -104.9903},
                                             {"Hartford", "CT", 41.7658, -72.6734},
                                             {"Dover", "DE", 39.1582, -75.5244},
                                             {"Tallahassee", "FL", 30.4383, -84.2807},
                                             {"Atlanta", "GA", 33.7490, -84.3880}};
                    for (int i = 0; i < 10; ++i) {
                        char tempval[16];
                        get_weather_for_location(capitals[i].city, capitals[i].state,
                                                 capitals[i].latitude, capitals[i].longitude,
                                                 tempval, sizeof(tempval));
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "{\"type\":\"weather_row\",\"city\":\"%s\",\"state\":\"%s\","
                                 "\"temp\":\"%s\"}",
                                 capitals[i].city, capitals[i].state, tempval);
                        int wsret = websocket_send_text(client, msg);
                        if (wsret < 0)
                            break;
                    }
                    weather_in_progress = 0;
                }
            }
        }
        close(client);
    } else {
        // Serve static file based on the requested path
        char method[8] = {0};
        char path[256] = {0};
        sscanf(request, "%7s %255s", method, path);
        http_serve_file(client, path);
        close(client);
    }
    free(ctx);
    return 0;
}

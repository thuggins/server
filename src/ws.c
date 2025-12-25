#include "ws_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#ifndef WS_GUID
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#endif

// Perform WebSocket handshake over plain socket.
int websocket_handshake(int client, const char* request) {
    char upgrade[128], conn[128], key[256];
    if (!get_header_value(request, "Upgrade", upgrade, sizeof(upgrade)) &&
        !get_header_value(request, "upgrade", upgrade, sizeof(upgrade)))
        return 0;
    if (!get_header_value(request, "Connection", conn, sizeof(conn)) &&
        !get_header_value(request, "connection", conn, sizeof(conn)))
        return 0;
    if (!get_header_value(request, "Sec-WebSocket-Key", key, sizeof(key)))
        return 0;
    if (!contains_case_insensitive(upgrade, "websocket"))
        return 0;
    if (!contains_case_insensitive(conn, "upgrade"))
        return 0;

    char to_hash[512];
    snprintf(to_hash, sizeof(to_hash), "%s%s", key, WS_GUID);
    unsigned char digest[20];
    if (!sha1_hash((const unsigned char*)to_hash, (uint32_t)strlen(to_hash), digest))
        return 0;
    char accept[64];
    if (base64_encode(digest, 20, accept, sizeof(accept)) <= 0)
        return 0;

    char response[512];
    int n = snprintf(response, sizeof(response),
                     "HTTP/1.1 101 Switching Protocols\r\n"
                     "Upgrade: websocket\r\n"
                     "Connection: Upgrade\r\n"
                     "Sec-WebSocket-Accept: %s\r\n"
                     "\r\n",
                     accept);
    send(client, response, n, 0);
    return 1;
}

// Send a text message over WebSocket (plain socket)
int websocket_send_text(int client, const char* msg) {
    size_t len = strlen(msg);
    unsigned char frame[2 + 125 + 2];
    size_t o = 0;
    frame[o++] = 0x81;
    if (len <= 125) {
        frame[o++] = (unsigned char)len;
    } else {
        return -1;
    }
    memcpy(frame + o, msg, len);
    o += len;
    return send(client, frame, (int)o, 0);
}

// Read a text message from WebSocket (plain socket). Handles ping/pong and close frames.
int websocket_read_text(int client, char* out, int out_size) {
    unsigned char hdr[2];
    int r = recv(client, (char*)hdr, 2, 0);
    if (r != 2)
        return -1;
    unsigned char opcode = hdr[0] & 0x0F;
    unsigned char masked = (hdr[1] & 0x80) ? 1 : 0;
    size_t len = hdr[1] & 0x7F;
    if (opcode == 0x8)
        return 0;
    if (opcode == 0x9) {
        if (!masked)
            return -1;
        if (len == 126) {
            unsigned char ext[2];
            if (recv(client, (char*)ext, 2, 0) != 2)
                return -1;
            len = (ext[0] << 8) | ext[1];
        } else if (len == 127) {
            return -1;
        }
        unsigned char mask[4];
        if (recv(client, (char*)mask, 4, 0) != 4)
            return -1;
        unsigned char* payload = (unsigned char*)malloc(len);
        if (!payload)
            return -1;
        size_t got = 0;
        while (got < len) {
            int chunk = recv(client, (char*)payload + got, (int)(len - got), 0);
            if (chunk <= 0) {
                free(payload);
                return -1;
            }
            got += chunk;
        }
        for (size_t i = 0; i < len; ++i)
            payload[i] ^= mask[i % 4];
        unsigned char frame_hdr[2];
        frame_hdr[0] = 0x8A; // FIN + pong
        if (len <= 125) {
            frame_hdr[1] = (unsigned char)len;
            send(client, (const char*)frame_hdr, 2, 0);
            if (len)
                send(client, (const char*)payload, (int)len, 0);
        }
        free(payload);
        return -2;
    }
    if (!masked)
        return -1;
    if (len == 126) {
        unsigned char ext[2];
        if (recv(client, (char*)ext, 2, 0) != 2)
            return -1;
        len = (ext[0] << 8) | ext[1];
    } else if (len == 127) {
        return -1;
    }
    unsigned char mask[4];
    if (recv(client, (char*)mask, 4, 0) != 4)
        return -1;
    if (len >= (size_t)out_size)
        return -1;
    unsigned char* payload = (unsigned char*)malloc(len);
    if (!payload)
        return -1;
    size_t got = 0;
    while (got < len) {
        int chunk = recv(client, (char*)payload + got, (int)(len - got), 0);
        if (chunk <= 0) {
            free(payload);
            return -1;
        }
        got += chunk;
    }
    for (size_t i = 0; i < len; ++i) {
        payload[i] ^= mask[i % 4];
    }
    memcpy(out, payload, len);
    out[len] = '\0';
    free(payload);
    return (int)len;
}

#include <openssl/ssl.h>

// Forward declarations for helpers/macros used in SSL functions
#ifndef WS_GUID
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#endif
static int base64_encode(const unsigned char* in, int in_len, char* out, int out_size);
static int sha1_hash(const unsigned char* data, DWORD len, unsigned char* out20);
static int get_header_value(const char* request, const char* name, char* out, int out_size);
static int contains_case_insensitive(const char* haystack, const char* needle);
#include <openssl/ssl.h>
int websocket_handshake_ssl(SSL* ssl, const char* request) {
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
    if (!sha1_hash((const unsigned char*)to_hash, (DWORD)strlen(to_hash), digest))
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
    SSL_write(ssl, response, n);
    return 1;
}

int websocket_send_text_ssl(SSL* ssl, const char* msg) {
    size_t len = strlen(msg);
    unsigned char frame[2 + 125 + 2];
    size_t o = 0;
    frame[o++] = 0x81; // FIN + text frame
    if (len <= 125) {
        frame[o++] = (unsigned char)len; // no mask from server
    } else {
        return -1; // keep simple
    }
    memcpy(frame + o, msg, len);
    o += len;
    return SSL_write(ssl, frame, (int)o);
}

int websocket_read_text_ssl(SSL* ssl, char* out, int out_size) {
    unsigned char hdr[2];
    int r = SSL_read(ssl, (char*)hdr, 2);
    if (r != 2)
        return -1;
    unsigned char opcode = hdr[0] & 0x0F;
    unsigned char masked = (hdr[1] & 0x80) ? 1 : 0;
    size_t len = hdr[1] & 0x7F;
    if (opcode == 0x8)
        return 0; // close
    // handle ping (0x9): read payload and reply with pong
    if (opcode == 0x9) {
        if (!masked)
            return -1;
        if (len == 126) {
            unsigned char ext[2];
            if (SSL_read(ssl, (char*)ext, 2) != 2)
                return -1;
            len = (ext[0] << 8) | ext[1];
        } else if (len == 127) {
            return -1;
        }
        unsigned char mask[4];
        if (SSL_read(ssl, (char*)mask, 4) != 4)
            return -1;
        unsigned char* payload = (unsigned char*)malloc(len);
        if (!payload)
            return -1;
        size_t got = 0;
        while (got < len) {
            int chunk = SSL_read(ssl, (char*)payload + got, (int)(len - got));
            if (chunk <= 0) {
                free(payload);
                return -1;
            }
            got += chunk;
        }
        for (size_t i = 0; i < len; ++i)
            payload[i] ^= mask[i % 4];
        // send pong frame (server frames are not masked)
        unsigned char frame_hdr[2];
        frame_hdr[0] = 0x8A; // FIN + pong
        if (len <= 125) {
            frame_hdr[1] = (unsigned char)len;
            SSL_write(ssl, (const char*)frame_hdr, 2);
            if (len)
                SSL_write(ssl, (const char*)payload, (int)len);
        }
        free(payload);
        return -2; // indicate control frame handled
    }
    if (!masked)
        return -1; // client-to-server must be masked
    if (len == 126) {
        unsigned char ext[2];
        if (SSL_read(ssl, (char*)ext, 2) != 2)
            return -1;
        len = (ext[0] << 8) | ext[1];
    } else if (len == 127) {
        return -1; // too big for demo
    }
    unsigned char mask[4];
    if (SSL_read(ssl, (char*)mask, 4) != 4)
        return -1;
    if (len >= (size_t)out_size)
        return -1;
    unsigned char* payload = (unsigned char*)malloc(len);
    if (!payload)
        return -1;
    size_t got = 0;
    while (got < len) {
        int chunk = SSL_read(ssl, (char*)payload + got, (int)(len - got));
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
/**
@file ws.c
@brief Minimal RFC 6455 WebSocket server utilities.

Provides:
- Handshake (Sec-WebSocket-Accept)
- Text frame send/receive
- Ping/Pong handling
@note No fragmentation; small payloads only.
*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <wincrypt.h>

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/**
@brief Encode raw bytes to Base64 for header values.
@param in Input buffer.
@param in_len Input length in bytes.
@param out Output buffer for Base64 string.
@param out_size Size of output buffer in bytes.
@return Number of chars written (excluding NUL) or -1 on overflow.
*/
static int base64_encode(const unsigned char* in, int in_len, char* out, int out_size) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int o = 0;
    for (int i = 0; i < in_len; i += 3) {
        unsigned int val = (in[i] << 16) & 0xFF0000;
        if (i + 1 < in_len)
            val |= (in[i + 1] << 8) & 0x00FF00;
        if (i + 2 < in_len)
            val |= (in[i + 2]) & 0x0000FF;

        if (o + 4 >= out_size)
            return -1;
        out[o++] = tbl[(val >> 18) & 0x3F];
        out[o++] = tbl[(val >> 12) & 0x3F];
        out[o++] = (i + 1 < in_len) ? tbl[(val >> 6) & 0x3F] : '=';
        out[o++] = (i + 2 < in_len) ? tbl[val & 0x3F] : '=';
    }
    if (o >= out_size)
        return -1;
    out[o] = '\0';
    return o;
}

/**
@brief Compute SHA-1 digest using Windows CryptoAPI.
@param data Input bytes.
@param len Length of input in bytes.
@param out20 Receives 20-byte digest.
@return 1 on success, 0 on failure.
*/
static int sha1_hash(const unsigned char* data, DWORD len, unsigned char* out20) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    DWORD hashLen = 20;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        return 0;
    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return 0;
    }
    if (!CryptHashData(hHash, data, len, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return 0;
    }
    if (!CryptGetHashParam(hHash, HP_HASHVAL, out20, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return 0;
    }
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return 1;
}

/**
@brief Extract a header value from an HTTP request string.
@param request Full HTTP request string.
@param name Header name to search for.
@param out Output buffer for header value.
@param out_size Size of `out` buffer.
@return 1 if found and copied, 0 if not found.
*/
static int get_header_value(const char* request, const char* name, char* out, int out_size) {
    size_t nlen = strlen(name);
    const char* p = request;
    while ((p = strstr(p, name)) != NULL) {
        if ((p == request || *(p - 1) == '\n') && strncmp(p, name, nlen) == 0 && p[nlen] == ':') {
            p += nlen + 1; // skip name and ':'
            while (*p == ' ')
                p++;
            const char* end = strstr(p, "\r\n");
            int len = end ? (int)(end - p) : (int)strlen(p);
            if (len >= out_size)
                len = out_size - 1;
            strncpy(out, p, len);
            out[len] = '\0';
            return 1;
        }
        p += nlen;
    }
    return 0;
}

/**
@brief Case-insensitive substring check (ASCII).
@param haystack Full string to search.
@param needle Search token.
@return Non-zero if found, 0 otherwise.
*/
static int contains_case_insensitive(const char* haystack, const char* needle) {
    char h[256], n[64];
    int hl = (int)strlen(haystack);
    int nl = (int)strlen(needle);
    if (hl >= (int)sizeof(h))
        hl = (int)sizeof(h) - 1;
    if (nl >= (int)sizeof(n))
        nl = (int)sizeof(n) - 1;
    strncpy(h, haystack, hl);
    h[hl] = '\0';
    strncpy(n, needle, nl);
    n[nl] = '\0';
    for (int i = 0; h[i]; ++i)
        h[i] = (char)tolower((unsigned char)h[i]);
    for (int i = 0; n[i]; ++i)
        n[i] = (char)tolower((unsigned char)n[i]);
    return strstr(h, n) != NULL;
}

/**
@brief Validate upgrade headers, compute Sec-WebSocket-Accept, and reply 101.
*/
int websocket_handshake(SOCKET client, const char* request) {
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
    if (!sha1_hash((const unsigned char*)to_hash, (DWORD)strlen(to_hash), digest))
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

/**
@brief Send a single unmasked text frame to the client.
*/
int websocket_send_text(SOCKET client, const char* msg) {
    size_t len = strlen(msg);
    unsigned char frame[2 + 125 + 2];
    size_t o = 0;
    frame[o++] = 0x81; // FIN + text frame
    if (len <= 125) {
        frame[o++] = (unsigned char)len; // no mask from server
    } else {
        return -1; // keep simple
    }
    memcpy(frame + o, msg, len);
    o += len;
    return send(client, (const char*)frame, (int)o, 0);
}

/**
@brief Read a masked client frame; handle ping/pong; return text payload.
*/
int websocket_read_text(SOCKET client, char* out, int out_size) {
    unsigned char hdr[2];
    int r = recv(client, (char*)hdr, 2, 0);
    if (r != 2)
        return -1;
    unsigned char opcode = hdr[0] & 0x0F;
    unsigned char masked = (hdr[1] & 0x80) ? 1 : 0;
    size_t len = hdr[1] & 0x7F;
    if (opcode == 0x8)
        return 0; // close
    // handle ping (0x9): read payload and reply with pong
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
        // send pong frame (server frames are not masked)
        unsigned char frame_hdr[2];
        frame_hdr[0] = 0x8A; // FIN + pong
        if (len <= 125) {
            frame_hdr[1] = (unsigned char)len;
            send(client, (const char*)frame_hdr, 2, 0);
            if (len)
                send(client, (const char*)payload, (int)len, 0);
        }
        free(payload);
        return -2; // indicate control frame handled
    }
    if (!masked)
        return -1; // client-to-server must be masked
    if (len == 126) {
        unsigned char ext[2];
        if (recv(client, (char*)ext, 2, 0) != 2)
            return -1;
        len = (ext[0] << 8) | ext[1];
    } else if (len == 127) {
        return -1; // too big for demo
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

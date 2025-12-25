// Minimal public domain SHA-1 implementation for WebSocket handshake
// Source: https://github.com/clibs/sha1 (public domain)
#include "sha1.h"
#include <string.h>

#define SHA1_BLOCK_SIZE 20

#define ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

static void sha1_transform(uint32_t state[5], const unsigned char buffer[64]) {
    uint32_t a, b, c, d, e, t, i, w[80];
    for (i = 0; i < 16; ++i) {
        w[i] = ((uint32_t)buffer[i * 4 + 0] << 24) | ((uint32_t)buffer[i * 4 + 1] << 16) |
               ((uint32_t)buffer[i * 4 + 2] << 8) | ((uint32_t)buffer[i * 4 + 3]);
    }
    for (; i < 80; ++i) {
        w[i] = ROL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    for (i = 0; i < 80; ++i) {
        if (i < 20)
            t = ((b & c) | ((~b) & d)) + 0x5A827999;
        else if (i < 40)
            t = (b ^ c ^ d) + 0x6ED9EBA1;
        else if (i < 60)
            t = ((b & c) | (b & d) | (c & d)) + 0x8F1BBCDC;
        else
            t = (b ^ c ^ d) + 0xCA62C1D6;
        t += ROL(a, 5) + e + w[i];
        e = d;
        d = c;
        c = ROL(b, 30);
        b = a;
        a = t;
    }
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

void sha1(const unsigned char* data, size_t len, unsigned char* out20) {
    uint32_t state[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    unsigned char buffer[64];
    size_t i, j;
    uint64_t total_bits = len * 8;
    for (i = 0; i + 64 <= len; i += 64) {
        sha1_transform(state, data + i);
    }
    j = len - i;
    memcpy(buffer, data + i, j);
    buffer[j++] = 0x80;
    if (j > 56) {
        memset(buffer + j, 0, 64 - j);
        sha1_transform(state, buffer);
        memset(buffer, 0, 56);
    } else {
        memset(buffer + j, 0, 56 - j);
    }
    uint64_t bits_be = ((uint64_t)(total_bits) >> 56) |
                       (((uint64_t)(total_bits) & 0x00FF000000000000) >> 40) |
                       (((uint64_t)(total_bits) & 0x0000FF0000000000) >> 24) |
                       (((uint64_t)(total_bits) & 0x000000FF00000000) >> 8) |
                       (((uint64_t)(total_bits) & 0x00000000FF000000) << 8) |
                       (((uint64_t)(total_bits) & 0x0000000000FF0000) << 24) |
                       (((uint64_t)(total_bits) & 0x000000000000FF00) << 40) |
                       (((uint64_t)(total_bits) & 0x00000000000000FF) << 56);
    memcpy(buffer + 56, &bits_be, 8);
    sha1_transform(state, buffer);
    for (i = 0; i < 5; ++i) {
        out20[i * 4 + 0] = (state[i] >> 24) & 0xFF;
        out20[i * 4 + 1] = (state[i] >> 16) & 0xFF;
        out20[i * 4 + 2] = (state[i] >> 8) & 0xFF;
        out20[i * 4 + 3] = (state[i]) & 0xFF;
    }
}

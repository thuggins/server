// Minimal public domain SHA-1 implementation for WebSocket handshake
// Source: https://github.com/clibs/sha1 (public domain)
// Adapted for use in this project

#ifndef SHA1_H
#define SHA1_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void sha1(const unsigned char* data, size_t len, unsigned char* out20);

#ifdef __cplusplus
}
#endif

#endif // SHA1_H

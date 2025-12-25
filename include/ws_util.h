#ifndef WS_UTIL_H
#define WS_UTIL_H

#include <stddef.h>
#include <stdint.h>

int base64_encode(const unsigned char* in, int in_len, char* out, int out_size);
int sha1_hash(const unsigned char* data, uint32_t len, unsigned char* out20);
int get_header_value(const char* request, const char* name, char* out, int out_size);
int contains_case_insensitive(const char* haystack, const char* needle);

#endif // WS_UTIL_H

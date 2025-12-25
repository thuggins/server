#include "ws_util.h"
#include "sha1.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Encode raw bytes to Base64 for header values.
int base64_encode(const unsigned char* in, int in_len, char* out, int out_size) {
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

// Compute SHA-1 digest using portable implementation.
int sha1_hash(const unsigned char* data, uint32_t len, unsigned char* out20) {
    sha1(data, len, out20);
    return 1;
}

// Extract a header value from an HTTP request string.
int get_header_value(const char* request, const char* name, char* out, int out_size) {
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

// Case-insensitive substring check (ASCII).
int contains_case_insensitive(const char* haystack, const char* needle) {
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

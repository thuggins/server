#include <ctype.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http_client_openssl.h"
#include "weather.h"

// Simple chunked transfer decoding: input is the chunked body, output is the decoded body
// (in-place) Returns length of decoded body, or -1 on error
static int decode_chunked_body(char* body) {
    char* src = body;
    char* dst = body;
    while (1) {
        // Read chunk size (hex)
        while (*src && isspace((unsigned char)*src))
            src++;
        char* size_start = src;
        int chunk_size = (int)strtol(size_start, &src, 16);
        if (chunk_size == 0)
            break;
        while (*src == '\r' || *src == '\n')
            src++;
        // Copy chunk data
        for (int i = 0; i < chunk_size && *src; ++i)
            *dst++ = *src++;
        // Skip CRLF after chunk
        while (*src == '\r' || *src == '\n')
            src++;
    }
    *dst = 0;
    return (int)(dst - body);
}

// Fetch weather for a location and write temperature as string to tempval. Returns 1 on success, 0
// on error. city and state are for display only; latitude and longitude are used for the API
// request.
int get_weather_for_location(const char* city, const char* state, double latitude, double longitude,
                             char* tempval, int tempval_size) {
    char response[8192] = {0};
    char path[256];
    snprintf(path, sizeof(path), "/v1/forecast?latitude=%.4f&longitude=%.4f&current_weather=true",
             latitude, longitude);
    int n = fetch_https("api.open-meteo.com", path, response, sizeof(response) - 1);
    if (n > 0) {
        char* json = strstr(response, "\r\n\r\n");
        if (json)
            json += 4;
        else
            json = response;
        // Decode chunked transfer encoding if present
        if (strstr(response, "Transfer-Encoding: chunked") != NULL) {
            decode_chunked_body(json);
        }
        char* cw = strstr(json, "\"current_weather\":");
        if (cw) {
            char* temp = strstr(cw, "\"temperature\":");
            if (temp) {
                double t = atof(temp + 14);
                snprintf(tempval, tempval_size, "%.1f", t);
                return 1;
            }
        }
    }
    snprintf(tempval, tempval_size, "?");
    return 0;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "weather.h"
#include <curl/curl.h>

// Helper for libcurl to write response data into a buffer
static size_t write_to_buffer(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    strncat((char*)userp, (const char*)contents, total);
    return total;
}

// Static CURL handle for connection reuse
static CURL* shared_curl = NULL;

// Call this at program start
void weather_curl_init() {
    if (!shared_curl) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        shared_curl = curl_easy_init();
    }
}

// Call this at program exit
void weather_curl_cleanup() {
    if (shared_curl) {
        curl_easy_cleanup(shared_curl);
        shared_curl = NULL;
        curl_global_cleanup();
    }
}

int get_weather_for_location(const char* city, const char* state, double latitude, double longitude,
                             char* tempval, int tempval_size) {
    char response[8192] = {0};
    char url[512];
    snprintf(
        url, sizeof(url),
        "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current_weather=true",
        latitude, longitude);

    if (!shared_curl) {
        snprintf(tempval, tempval_size, "?");
        return 0;
    }
    curl_easy_reset(shared_curl);
    curl_easy_setopt(shared_curl, CURLOPT_URL, url);
    curl_easy_setopt(shared_curl, CURLOPT_WRITEFUNCTION, write_to_buffer);
    curl_easy_setopt(shared_curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(shared_curl, CURLOPT_USERAGENT, "C-libcurl");
    CURLcode res = curl_easy_perform(shared_curl);
    if (res != CURLE_OK) {
        snprintf(tempval, tempval_size, "?");
        return 0;
    }
    // Find temperature in JSON
    char* cw = strstr(response, "\"current_weather\":");
    if (cw) {
        char* temp = strstr(cw, "\"temperature\":");
        if (temp) {
            double t = atof(temp + 14);
            snprintf(tempval, tempval_size, "%.1f", t);
            return 1;
        }
    }
    snprintf(tempval, tempval_size, "?");
    return 0;
}

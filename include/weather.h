#ifndef WEATHER_H
#define WEATHER_H

// Struct for a location (city, state, lat/lon)
typedef struct {
    const char* city;
    const char* state;
    double latitude;
    double longitude;
} Location;

// Call at program start to initialize libcurl for weather
void weather_curl_init();

// Call at program exit to clean up libcurl for weather
void weather_curl_cleanup();

// Fetches weather for a location and writes temperature as string to tempval.
// Returns 1 on success, 0 on error.
// city and state are for display only; latitude and longitude are used for the API request.
int get_weather_for_location(const char* city, const char* state, double latitude, double longitude,
                             char* tempval, int tempval_size);

#endif // WEATHER_H

#ifndef HTTP_CLIENT_OPENSSL_H
#define HTTP_CLIENT_OPENSSL_H

// Performs a minimal HTTPS GET request using OpenSSL and sockets.
// Returns number of bytes received, or -1 on error.
int fetch_https(const char* host, const char* path, char* out, int out_size);

#endif // HTTP_CLIENT_OPENSSL_H

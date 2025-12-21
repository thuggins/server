#ifndef HTTP_H
#define HTTP_H

#include <winsock2.h>

// Send a 200 OK response with the provided body.
void http_send_response_ssl(void* ssl, const char* content_type, const unsigned char* data,
                            size_t len);

// Send a simple 404 Not Found HTML response.
void http_send_404_ssl(void* ssl);

// Serve a file by HTTP path from public/.
void http_serve_file_ssl(void* ssl, const char* path);

#endif // HTTP_H

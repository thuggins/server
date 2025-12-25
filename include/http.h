#ifndef HTTP_H
#include <stddef.h>
#define HTTP_H

// Send a 200 OK response with the provided body.
void http_send_response(int client, const char* content_type, const unsigned char* data,
                        size_t len);

// Send a simple 404 Not Found HTML response.
void http_send_404(int client);

// Serve a file by HTTP path from public/.
void http_serve_file(int client, const char* path);

#endif // HTTP_H

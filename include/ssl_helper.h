#ifndef SSL_HELPER_H
#define SSL_HELPER_H

#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX* create_ssl_ctx(const char* cert_file, const char* key_file);

#endif // SSL_HELPER_H

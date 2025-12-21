#include "ssl_helper.h"
#include <stdio.h>

// Create and return a new SSL_CTX using the given cert and key files.
SSL_CTX* create_ssl_ctx(const char* cert_file, const char* key_file) {
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        fprintf(stderr, "SSL_CTX_new failed\n");
        return NULL;
    }
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "SSL_CTX_use_certificate_file failed\n");
        SSL_CTX_free(ctx);
        return NULL;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "SSL_CTX_use_PrivateKey_file failed\n");
        SSL_CTX_free(ctx);
        return NULL;
    }
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "SSL_CTX_check_private_key failed\n");
        SSL_CTX_free(ctx);
        return NULL;
    }
    return ctx;
}

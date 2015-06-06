#ifndef L_HTTP_UTIL_H

#include "common.h"

#define UV_CHECK(error)                                     \
    do {                                                    \
        int err = (error);                                  \
        if (err) {                                          \
            l_error("FATAL ERROR: %s", uv_strerror(err));   \
            exit(1);                                        \
        }                                                   \
    } while (0)

int l_has_error(const char *errmsg);

hitem_t *l_add_header(hitem_t *headers, const char *field, const char *value);
char *l_get_header(hitem_t *headers, const char *field);
void l_free_headers(hitem_t *headers);
void l_print_headers(hitem_t *headers);

#define L_HTTP_UTIL_H
#endif
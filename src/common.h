#ifndef L_COMMON_H

#include "../include/config.h"
#include "../include/http_parser.h"
#include "../include/uthash.h"
#include "../include/lhttpd.h"

#define UNINIT         -1
#define WAIT_FOR_VALUE -2

#define MAX_STATUS_CODE_NUM 1000
#define MAX_HTTP_METHOD_NUM 100

#define UV_CHECK(error)                                     \
    do {                                                    \
        int err = (error);                                  \
        if (err) {                                          \
            l_error("FATAL ERROR: %s", uv_strerror(err));   \
            exit(1);                                        \
        }                                                   \
    } while (0)

#define L_COMMON_H
#endif
#ifndef _COMMON_H

#include "../include/lhttpd.h"
#include "../config.h"

#define UNINIT         -1
#define WAIT_FOR_VALUE -2

#define UV_CHECK(error)                                     \
    do {                                                    \
        int err = (error);                                  \
        if (err) {                                          \
            l_error("FATAL ERROR: %s", uv_strerror(err));   \
            exit(1);                                        \
        }                                                   \
    } while (0)

#define _COMMON_H
#endif
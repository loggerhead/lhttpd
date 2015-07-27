#ifndef _SERVER_H
#define _SERVER_H

#include "common.h"

#define UV_CHECK(error)                                     \
    do {                                                    \
        int err = (error);                                  \
        if (err) {                                          \
            l_error("FATAL ERROR: %s", uv_strerror(err));   \
            exit(1);                                        \
        }                                                   \
    } while (0)

#endif /* _SERVER_H */

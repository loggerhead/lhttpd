#ifndef _HTTP_UTIL_H
#define _HTTP_UTIL_H

#include "common.h"

#define IMPLEMENTED_HTTP_METHOD_MAP(XX)     \
    XX(GET)                                 \
    XX(PUT)                                 \
    XX(POST)                                \
    XX(HEAD)                                \
    XX(DELETE)

#endif /* _HTTP_UTIL_H */
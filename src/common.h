#ifndef _COMMON_H
#define _COMMON_H

#include "../include/config.h"
#include "../include/http_parser.h"
#include "../include/uthash.h"
#include "../include/lhttpd.in.h"

#define MAX_STATUS_CODE_NUM 1000
#define MAX_HTTP_METHOD_NUM 100
#define MAX_ERROR_CODE 100

#define ERR_MSG_MAP(XX)                                                      \
    XX(ERR_NONE,               0, NULL)                                      \
    XX(ERR_CREATE_CONNECTION,  1, "create connection failed")                \
    XX(ERR_UV_READ,            2, "incorrect read on libuv")                 \
    XX(ERR_HTTP_PARSE,         3, "parse HTTP request failed")               \
    XX(ERR_MEMORY_ALLOC,       4, "can't alloc enough memory")               \
    XX(ERR_UV_WRITE,           5, "send bytes to client by libuv failed")    \
    XX(ERR_MIMETYPES,          6, "can't find mimetypes file")               \
    XX(ERR_GUARD,             99, NULL)

enum {
#define XX(macro, code, message) macro = -code,
    ERR_MSG_MAP(XX)
#undef XX
};

#define LOG_ERROR_STR(errmsg) l_warn("%s - - [%s]: %s", __func__, l_now(), errmsg)

#define LOG_ERROR(err)                                            \
    do {                                                          \
        const char *errmsg = _strerror(err);                      \
        if (errmsg)                                               \
            LOG_ERROR_STR(errmsg);                                \
    } while (0)

#define _(ret)           \
    do {                 \
        err = (ret);     \
        if (err)         \
            goto END;    \
    } while (0)

const char *_strerror(int err);
void _log_request(l_client_t *client);

#endif /* _COMMON_H */
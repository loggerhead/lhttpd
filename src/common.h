#ifndef _COMMON_H

#include <uv.h>
#include "../config.h"
#include "http_parser.h"
#include "uthash.h"

#define UNINIT         -1
#define WAIT_FOR_VALUE -2

typedef struct hitem_t hitem_t;
typedef struct client_t client_t;
typedef struct server_t server_t;

typedef void         (*l_on_read_cb)    (uv_stream_t *, ssize_t, const uv_buf_t *);
typedef const char * (*l_on_data_cb)    (client_t *, const char *, ssize_t);
typedef const char * (*l_on_request_cb) (client_t *);

struct hitem_t {
    char *key;
    char *value;
    UT_hash_handle hh;
};

#define HOOK(field) (field->data)

struct client_t {
    uv_tcp_t handle;
    // DO NOT USE parser.content_length as content-length
    http_parser parser;

    int is_message_complete;

    char *url;
    hitem_t *headers;
    char *body;
    int content_length;
    int readed_len;

    server_t *server;
};

struct server_t {
    const char *ip;
    int port;
    uv_loop_t loop;
    uv_tcp_t handle;

    l_on_read_cb on_read_cb;
    l_on_data_cb on_data_cb;
    l_on_request_cb on_request_cb;
};

#define _COMMON_H
#endif
#ifndef _LHTTPD_H

#include <uv.h>
#include "http_parser.h"
#include "uthash.h"

typedef enum {
#ifndef FALSE
    FALSE,
#else
    L_FALSE,
#endif
#ifndef TRUE
    TRUE,
#else
    L_TRUE,
#endif
} l_bool_t;

#define CONVERT_PARSER_METHOD(parser_method) 1 << (parser_method)

typedef enum {
#define XX(num, name, string) L_HTTP_ ## name = CONVERT_PARSER_METHOD(num),
    HTTP_METHOD_MAP(XX)
#undef XX
    L_HTTP_UNKNOWN = 0,
} l_http_method_t;

typedef struct _hitem_t l_hitem_t;
typedef struct _client_t l_client_t;
typedef struct _server_t l_server_t;
typedef struct _http_request_t l_http_request_t;

typedef void         (*l_on_read_cb)    (uv_stream_t *, ssize_t, const uv_buf_t *);
typedef const char * (*l_on_data_cb)    (l_client_t *, const char *, ssize_t);
typedef const char * (*l_on_request_cb) (l_client_t *);

typedef void (*l_hitem_free_fn) (l_hitem_t *item);

struct _hitem_t {
    char *key;
    char *value;
    UT_hash_handle hh;
};

struct _http_request_t {
    l_http_method_t method;
    l_hitem_t *headers;
    char *url;
    char *body;
    long content_length;
    long body_nread;
    l_bool_t is_finished;
};

struct _client_t {
    uv_tcp_t handle;

    // DO NOT USE parser.content_length as content-length
    http_parser parser;
    l_http_request_t req;

    l_server_t *server;
};

struct _server_t {
    uv_tcp_t handle;
    uv_loop_t loop;

    const char *ip;
    int port;

    l_on_data_cb on_data_cb;
    l_on_request_cb on_request_cb;
};

// Server
l_server_t *l_create_server();
void l_set_ip_port(l_server_t *server, const char *ip, int port);
void l_start_server(l_server_t *server);

// Client
void l_client_reset(l_client_t *client);
l_client_t *l_create_client(l_server_t *server);
void l_close_connection(l_client_t *client);

const char *l_send_bytes(l_client_t *client, const char *bytes, size_t len);
const char *l_send_response(l_client_t *client, int status_code, l_hitem_t *headers, const char *body);
const char *l_send_code(l_client_t *client, int status_code);
const char *l_send_body(l_client_t *client, const char *body);

const char *l_status_code(int code);
char *l_generate_response(l_client_t *client, int status_code, l_hitem_t *headers, const char *body);

// HTTP Util
int l_has_error(const char *errmsg);

l_hitem_t *l_add_header(l_hitem_t *headers, const char *field, const char *value);
char *l_get_header(l_hitem_t *headers, const char *field);
void l_free_headers(l_hitem_t *headers);
void l_print_headers(l_hitem_t *headers);

void l_convert_parser_method(l_client_t *client);

l_bool_t l_is_http_get(l_client_t *client);
l_bool_t l_is_http_post(l_client_t *client);
l_bool_t l_is_http_put(l_client_t *client);
l_bool_t l_is_http_head(l_client_t *client);
l_bool_t l_is_http_delete(l_client_t *client);

// Util
#if DEBUG
# define L_TRACE(...)   l_log("--- %s ---", __func__)
#else
# define L_TRACE(...)
#endif /* DEBUG */

void l_error(const char *format, ...);
void l_warn(const char *format, ...);
void l_log(const char *format, ...);

#define L_FREE(memory) free((void *) (memory))
void *l_malloc(size_t size);
void *l_calloc(size_t count, size_t size);
void *l_realloc(void *ptr, size_t size);

char *l_mprintf(const char *fmt, ...);
char *l_strdup(const char *s);
char *l_strnchr(const char *s, char ch, size_t n);
void l_repchr(char *str, char old, char new);
void l_lowercase(char *str);

l_hitem_t *l_hput(l_hitem_t *hashtbl, const char *key, const char *value);
char *l_hget(l_hitem_t *hashtbl, const char *key);
// leave `free_fn` NULL to tell `l_hfree` not free fields
void l_hfree(l_hitem_t *hashtbl, l_hitem_free_fn free_fn);

#define L_HITER(hashtbl, item) for(item=hashtbl; item; item=item->hh.next)

#define _LHTTPD_H
#endif
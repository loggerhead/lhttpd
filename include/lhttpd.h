#ifndef _LHTTPD_H

#include <uv.h>
#include "http_parser.h"
#include "uthash.h"

/* Variable types */
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

typedef enum http_method l_http_method_t;

typedef struct _hitem_t l_hitem_t;
typedef struct _client_t l_client_t;
typedef struct _server_t l_server_t;
typedef struct _http_request_t l_http_request_t;
typedef struct _http_response_t l_http_response_t;

/* Callable */
// return error message
typedef const char * (*l_on_data_cb)    (l_client_t *, const char *, ssize_t);
typedef const char * (*l_on_request_cb) (l_client_t *);
typedef l_http_response_t (*l_match_router_cb) (l_client_t *);

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

struct _http_response_t {
    int status_code;
    l_hitem_t *headers;
    const char *body;
};


struct _client_t {
    uv_tcp_t handle;
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

/******************************************************************************
** Routing
******************************************************************************/
void l_add_router(const char *rule, l_http_method_t method, l_match_router_cb callback);
l_match_router_cb l_match_router(const char *url, l_http_method_t method);

/******************************************************************************
** Server
******************************************************************************/
l_server_t *l_create_server();
void l_set_ip_port(l_server_t *server, const char *ip, int port);
void l_start_server(l_server_t *server);

/******************************************************************************
** Client
******************************************************************************/
void l_client_reset(l_client_t *client);
l_client_t *l_create_client(l_server_t *server);
void l_close_connection(l_client_t *client);

l_http_response_t l_create_response();
char *l_generate_response_str(l_client_t *client, l_http_response_t response);

/******************************************************************************
** Data transmission
******************************************************************************/
const char *l_send_bytes(l_client_t *client, const char *bytes, size_t len);
const char *l_send_response(l_client_t *client, l_http_response_t response);
const char *l_send_code(l_client_t *client, int status_code);
const char *l_send_body(l_client_t *client, const char *body);

/******************************************************************************
** HTTP util
******************************************************************************/
int l_has_error(const char *errmsg);

const char *l_status_code_str(int status_code);
char *l_generate_response_str(l_client_t *client, l_http_response_t response);

l_hitem_t *l_add_header(l_hitem_t *headers, const char *field, const char *value);
char *l_get_header(l_hitem_t *headers, const char *field);
void l_free_headers(l_hitem_t *headers);
void l_print_headers(l_hitem_t *headers);

l_bool_t l_is_implemented_http_method(l_client_t *client);
l_bool_t l_is_http_get(l_client_t *client);
l_bool_t l_is_http_post(l_client_t *client);
l_bool_t l_is_http_put(l_client_t *client);
l_bool_t l_is_http_head(l_client_t *client);
l_bool_t l_is_http_delete(l_client_t *client);

/******************************************************************************
** Logging facility
******************************************************************************/
#if DEBUG
# define L_TRACE(...)   l_log("--- %s ---", __func__)
#else
# define L_TRACE(...)
#endif /* DEBUG */
void l_error(const char *format, ...);
void l_warn(const char *format, ...);
void l_log(const char *format, ...);

/******************************************************************************
** Memory alloc and free
******************************************************************************/
#define L_FREE(memory) free((void *) (memory))
void *l_malloc(size_t size);
void *l_calloc(size_t count, size_t size);
void *l_realloc(void *ptr, size_t size);

/******************************************************************************
** String operation
******************************************************************************/
char *l_mprintf(const char *fmt, ...);
char *l_strdup(const char *s);
char *l_strnchr(const char *s, char ch, size_t n);
void l_repchr(char *str, char old, char new);
void l_lowercase(char *str);

/******************************************************************************
** Hash table
******************************************************************************/
#define L_HITER(hashtbl, item) for(item=hashtbl; item; item=item->hh.next)
l_hitem_t *l_hput(l_hitem_t *hashtbl, const char *key, const char *value);
char *l_hget(l_hitem_t *hashtbl, const char *key);
// leave `free_fn` NULL to tell `l_hfree` not free fields
void l_hfree(l_hitem_t *hashtbl, l_hitem_free_fn free_fn);

#define _LHTTPD_H
#endif
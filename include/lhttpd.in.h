/*
 * This file is used for generating `lhttpd.h` file.
 */
#ifndef _LHTTPD_H

#include <stdarg.h>
#include <uv.h>

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
typedef struct _route_match_t l_route_match_t;

/* Callable */
// return error message
typedef const char * (*l_on_data_cb)    (l_client_t *, const char *, ssize_t);
typedef const char * (*l_on_request_cb) (l_client_t *);
typedef l_http_response_t (*l_match_route_cb) (l_client_t *, l_hitem_t *args);

typedef void (*l_hitem_free_fn) (l_hitem_t *item);

/******************************************************************************
** Routing
******************************************************************************/
struct _route_match_t{
    l_match_route_cb callback;
    l_hitem_t *args;
};

void l_add_route(const char *route, l_http_method_t method, l_match_route_cb callback);
l_route_match_t l_match_route(const char *url, l_http_method_t method);

/******************************************************************************
** Server
******************************************************************************/
struct _server_t {
    uv_tcp_t handle;
    uv_loop_t loop;
    const char *ip;
    int port;
    l_on_data_cb on_data_cb;
    l_on_request_cb on_request_cb;
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

l_server_t *l_create_server();
void l_set_ip_port(l_server_t *server, const char *ip, int port);
void l_start_server(l_server_t *server);

/******************************************************************************
** Client
******************************************************************************/
struct _client_t {
    uv_tcp_t handle;
    http_parser parser;
    l_http_request_t req;
    l_server_t *server;
};

struct _http_response_t {
    int status_code;
    l_hitem_t *headers;
    const char *body;
};

l_client_t *l_create_client(l_server_t *server);
void l_client_reset(l_client_t *client);
void l_close_connection(l_client_t *client);

l_http_response_t l_create_response();
char *l_generate_response_str(l_client_t *client, l_http_response_t response);
const char *l_status_code_str(int status_code);

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
int l_is_num(const char *str);
int l_has_str(const char *str);
char *l_mprintf(const char *fmt, ...);
char *l_strdup(const char *s);
char *l_strnchr(const char *s, char ch, size_t n);
void l_repchr(char *str, char old, char new);
void l_lowercase(char *str);

/******************************************************************************
** Hash table
******************************************************************************/
struct _hitem_t {
    char *key;
    char *value;
    UT_hash_handle hh;
};

#define L_HITER(hashtbl, item) for(l_hitem_t *item=hashtbl; item; item=item->hh.next)
/* NOTE: `hashtbl` can NOT be a copy variable, for example:

        void error_example() {
            l_hitem_t *hashtbl = NULL;
            l_hitem_t *counterpart = hashtbl;
            l_hput(hashtbl, "foo", "bar");
            l_hput(counterpart, "foo", "another");
            assert(!strcmp(l_hget(hashtbl, "foo"), "bar"));
        }

 * `value` must be `integer number` or `pointer`, can NOT be `float number` or `struct`
 */
#define L_HPUT(hashtbl, key, value) (hashtbl=l_hput(hashtbl, key, (const char *) value))
l_hitem_t *l_hput(l_hitem_t *hashtbl, const char *key, const char *value);
char *l_hget(l_hitem_t *hashtbl, const char *key);
// leave `free_fn` NULL to tell `l_hfree` not free fields
void l_hfree(l_hitem_t *hashtbl, l_hitem_free_fn free_fn);

/******************************************************************************
** SQLite 3
******************************************************************************/
#if HAS_SQLITE3

#include <sqlite3.h>

typedef struct _query_t l_query_t;
typedef sqlite3 * l_db_t;

struct _query_t {
    int row;
    int col;
    char **results;
    l_bool_t is_success;
};

/* For example, print out first row of query result.
 *
 *    l_query_t *query = l_query_db(db, "select * from foo");
 *    char *key, *val;
 *    L_DB_FOREACH_COL(query, 0, key, val) {
 *        printf("%s: %s\n", key, val);
 *    }
 */
#define L_DB_FOREACH_COL(query, row, key, val)                     \
    for (int k = 0;                                                \
            ({ if(k < query->col && query->results) {              \
                key = query->results[k];                           \
                val = query->results[k + (row + 1) * query->col];  \
               }; k < query->col; });                              \
         k++)

/* For example, print out every row of query result.
 *
 *    l_query_t *query = l_query_db(db, "select * from foo");
 *    char *key, *val;
 *    L_DB_FOREACH_ROW(query, key, val) {
 *        printf("%s: %s\n", key, val);
 *    }
 */
#define L_DB_FOREACH_ROW(query, key, val)                          \
    for (int r = 0; query && r < query->row; r++)                  \
        L_DB_FOREACH_COL(query, r, key, val)

l_db_t l_create_db(const char *dbpath);
void l_close_db(l_db_t db);
void l_free_query(l_query_t *query);

/* NOTE: For below functions, an segmentation fault will occurred
 * when `sqlfmt` not consistent with args. This is a bug of `sqlite3_vmprintf`.
 * Get more detail from `https://www.sqlite.org/c3ref/mprintf.html`
 */

l_bool_t l_exec_db(l_db_t db, const char *sqlfmt, ...);
/* NOTE: result need free by `l_free_query`, for example.
 *     l_query_t *query = l_query_db(db, 'select 1');
 *     do_something(query);
 *     l_free_query(query);
 */
l_query_t *l_query_db(l_db_t db, const char *sqlfmt, ...);
l_bool_t l_is_exist_db(l_db_t db, const char *sqlfmt, ...);

#endif /* HAS_SQLITE3 */

/******************************************************************************
** Redis
******************************************************************************/
#if HAS_HIREDIS

#include <hiredis/hiredis.h>

typedef struct _redis_connection_t l_redis_connection_t;
typedef enum { L_SYNC, L_ASYNC } l_connection_type_t;

struct _redis_connection_t {
    redisContext *conn;
    l_connection_type_t type;
};

l_redis_connection_t l_create_redis_connection(const char *host, int port);
void l_close_redis_connection(l_redis_connection_t conn);

void l_redis_set(l_redis_connection_t conn, const char *key, const char *value);
// NOTE: return value need free
const char *l_redis_get(l_redis_connection_t conn, const char *key);

#endif /* HAS_HIREDIS */


#define _LHTTPD_H
#endif
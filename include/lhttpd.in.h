/*
 * This file is used for generating `lhttpd.h` file.
 */
#ifndef L_LHTTPD_H
#define L_LHTTPD_H

#include <stdio.h>
#include <stdarg.h>
#include <uv.h>


/* Variable types */
typedef enum {
#ifndef FALSE
    FALSE,
#define L_FALSE 0
#else
    L_FALSE,
#endif
#ifndef TRUE
    TRUE,
#define L_TRUE 1
#else
    L_TRUE,
#endif
} l_bool_t;


typedef enum http_method l_http_method_t;

typedef struct _buf_t l_buf_t;
typedef struct _hitem_t l_hitem_t;
typedef struct _client_t l_client_t;
typedef struct _server_t l_server_t;
typedef struct _http_request_t l_http_request_t;
typedef struct _http_response_t l_http_response_t;
typedef struct _route_match_t l_route_match_t;

/* Callable */
typedef void (*l_hitem_free_fn) (l_hitem_t *item);
typedef l_http_response_t (*l_match_route_cb) (l_client_t *, l_hitem_t *args);
// return error code
typedef int (*l_on_data_cb)    (l_client_t *, const char *, ssize_t);
typedef int (*l_on_request_cb) (l_client_t *);
typedef int (*l_after_response_cb) (l_http_response_t *);


/******************************************************************************
** webrouter.c
******************************************************************************/
struct _route_match_t {
    l_match_route_cb callback;
    l_hitem_t *args;
};

void l_add_route(const char *route, l_http_method_t method, l_match_route_cb callback);
l_route_match_t l_match_route(const char *url, l_http_method_t method);

/******************************************************************************
** server.c
******************************************************************************/
struct _server_t {
    uv_tcp_t handle;
    uv_loop_t loop;
    const char *ip;
    int port;
    l_on_data_cb on_data_cb;
    l_on_request_cb on_request_cb;
    long max_content_length;
};

struct _http_request_t {
    char *hook;
    l_http_method_t method;
    char *url;
    struct http_parser_url parsed_url;
    l_hitem_t *headers;
    char *body;
    long content_length;
    long body_nread;
    l_bool_t is_finished;
};

l_server_t *l_create_server();
void l_set_ip_port(l_server_t *server, const char *ip, int port);
void l_start_server(l_server_t *server);

/******************************************************************************
** httputil.c
******************************************************************************/
struct _buf_t {
    char *data;
    size_t len;
};

struct _http_response_t {
    int status_code;
    l_hitem_t *headers;
    const char *body;
    size_t body_len;
    l_after_response_cb callback;
};

struct _client_t {
    uv_tcp_t handle;
    http_parser parser;
    l_server_t *server;
    l_http_request_t request;
    l_http_response_t response;
    l_bool_t close_connection;
};

/* Connection */
l_client_t *l_create_connection(l_server_t *server);
void l_reset_connection(l_client_t *client);
void l_close_connection(l_client_t *client);

/* HTTP method */
l_bool_t l_is_implemented_http_method(l_client_t *client);
l_bool_t l_is_http_get(l_client_t *client);
l_bool_t l_is_http_post(l_client_t *client);
l_bool_t l_is_http_put(l_client_t *client);
l_bool_t l_is_http_head(l_client_t *client);
l_bool_t l_is_http_delete(l_client_t *client);

/* Data transmission */
int l_send_bytes(l_client_t *client, const char *bytes, size_t len);
int l_send_response_line(l_client_t *client, int http_minor, int status_code);
int l_send_header(l_client_t *client, const char *field, const char *value);
int l_send_blankline(l_client_t *client);
int l_send_response(l_client_t *client, l_http_response_t *response);
int l_send_code(l_client_t *client, int status_code);
int l_send_body(l_client_t *client, const char *body);

/* HTTP response */
const char *l_status_code_str(int status_code);
l_http_response_t l_create_response();
l_http_response_t l_create_redirect_response(const char *url);
/*
 * Create response from static file.
 * NOTE: filepath can't containing "..", otherwise, 404 will be returned.
 */
l_http_response_t l_create_response_by_file(l_client_t *client, const char *filepath);
void l_set_response_body(l_http_response_t *response, const char *body, size_t body_len);

/* HTTP header */
#define L_PUT_HEADER(headers, field, value)                 \
    do {                                                    \
        headers = l_put_header(headers, field, value);      \
    } while (0)

/* equivalent to `headers[field.lower()] = value`
 * NOTE: `l_put_header` will alloc memory for `field` and `value`
 */
l_hitem_t *l_put_header(l_hitem_t *headers, const char *field, const char *value);
// equivalent to `return headers[field.lower()]`
char *l_get_header(l_hitem_t *headers, const char *field);
void l_free_headers(l_hitem_t *headers);
void l_print_headers(l_hitem_t *headers);

/* return path part of url. For example, `www.test.com/foo` will return `/foo`
 * NOTE: need free
 */
const char *l_get_url_path(l_client_t *client);
const char *l_get_mimetype(const char *filepath);
// NOTE: need free
char *l_get_etag(const char *filepath);


/******************************************************************************
** util.c
******************************************************************************/
/* Logging facility */
#if DEBUG
# define L_TRACE(...)   l_log("--- %s ---", __func__)
#else
# define L_TRACE(...)
#endif /* DEBUG */
void l_error(const char *format, ...);
void l_warn(const char *format, ...);
void l_log(const char *format, ...);

/* Time */
const char *l_now();
// NOTE: need free
char *l_seconds2gmtime(time_t t);
char *l_gmtime();
time_t l_getmtime_seconds(const char *path);
char *l_getmtime(const char *path);

/* Memory alloc and free */
#define L_FREE(memory) free((void *) (memory))
void *l_malloc(size_t size);
void *l_calloc(size_t count, size_t size);
void *l_realloc(void *ptr, size_t size);

/* String operation */
l_bool_t l_is_num(const char *str);
l_bool_t l_is_str(const char *str);
l_bool_t l_is_streq(const char *str1, const char *str2);
l_bool_t l_is_strcaseeq(const char *str1, const char *str2);
char *l_mprintf(const char *fmt, ...);
char *l_strdup(const char *s);
char *l_strnchr(const char *s, char ch, size_t n);
void l_repchr(char *str, char old, char new);
void l_lowercase(char *str);

/* File operation */
l_bool_t l_match_file_suffix(const char *filename, const char *suffix);
l_bool_t l_is_file_exist(const char *path);
size_t l_get_filesize(const char *path);
size_t l_get_filesize_by_fp(FILE *fp);
// NOTE: return value need free
const char *l_pathcat(const char *dir, const char *filename);
// NOTE: return value need free by `L_FREE`
const char *l_url2filename(const char *url);
/* get the directory component of a pathname
 * NOTE: return value need free by `L_FREE`
 */
const char *l_get_dirname(const char *filepath);
// get the final component of a pathname
const char *l_get_basename(const char *filepath);
// get the suffix of filename
const char *l_get_suffix(const char *filename);
/* read file data into memory, return `buf.len` == 0 when failed
 * NOTE: `buf.data` need free by `L_FREE`
 */
l_buf_t l_read_file(const char *filepath);
void l_mkdirs(const char *dir);

uint32_t l_adler32(const char *data, size_t len);

/* Hash table */
struct _hitem_t {
    char *key;
    char *value;
    UT_hash_handle hh;
};

#define L_HITER(hashtbl, item) \
    for(l_hitem_t *item=hashtbl; item; item=item->hh.next)
/* NOTE: `hashtbl` can NOT be a copy variable, for example:
 *
 *     void error_example() {
 *         l_hitem_t *hashtbl = NULL;
 *         l_hitem_t *counterpart = hashtbl;
 *         l_hput(hashtbl, "foo", "bar");
 *         l_hput(counterpart, "foo", "another");
 *         assert(!strcmp(l_hget(hashtbl, "foo"), "bar"));
 *     }
 *
 * `value` must be `integer number` or `pointer`, can NOT be `float number` or `struct`
 */
#define L_HPUT(hashtbl, key, value) \
    do { hashtbl=l_hput(hashtbl, key, (const char *) value); } while (0)
l_hitem_t *l_hput(l_hitem_t *hashtbl, const char *key, const char *value);
char *l_hget(l_hitem_t *hashtbl, const char *key);
/* leave `free_fn` NULL to tell `l_hfree` not free fields
 * NOTE: just released all memory, so do NOT iterate `hashtbl` after called
 */
void l_hfree(l_hitem_t *hashtbl, l_hitem_free_fn free_fn);

/******************************************************************************
** json.c
******************************************************************************/
#if HAS_JSON_C

#include <json-c/json.h>

typedef struct _json_map_t l_json_map_t;
typedef json_object l_json_t;

struct _json_map_t {
    void *var;
    const char *key;
    json_type type;
};

#define L_JSON_MAP_END { NULL, NULL, json_type_null }

void l_json_load(l_json_map_t maps[], l_json_t *jobj);
// NOTE: result need free by `l_json_free` if no more need of `maps` variable
l_json_t *l_json_loads(l_json_map_t maps[], const char *jstr, size_t len);
// NOTE: result need free by `l_json_free`
l_json_t *l_json_dump(l_json_map_t maps[]);
// NOTE: return value need free
const char *l_json_dumps(l_json_map_t maps[]);

l_json_t *l_create_json_object();
l_json_t *l_create_json_array();
void l_free_json(l_json_t *jobj);
const char *l_json_to_string(l_json_t *jobj);

void l_json_add_string(l_json_t *jobj, const char *key, const char *val);
void l_json_add_double(l_json_t *jobj, const char *key, double val);
void l_json_add_int(l_json_t *jobj, const char *key, int val);
void l_json_add_bool(l_json_t *jobj, const char *key, l_bool_t val);
void l_json_add_jobj(l_json_t *jobj, const char *key, l_json_t *val);

void l_array_add_string(l_json_t *jobj, const char *val);
void l_array_add_double(l_json_t *jobj, double val);
void l_array_add_int(l_json_t *jobj, int val);
void l_array_add_bool(l_json_t *jobj, l_bool_t val);
void l_array_add_jobj(l_json_t *jobj, l_json_t *val);

#else
typedef void l_json_t;
#endif /* HAS_JSON_C */

/******************************************************************************
** sqlite.c
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
 *    L_DB_FOREACH_ROW(query, i) {
 *        L_DB_FOREACH_COL(query, i, key, val) {
 *            printf("%s: %s\n", key, val);
 *        }
 *    }
 */
#define L_DB_FOREACH_ROW(query, i)                          \
    for (int i = 0; query && i < query->row; i++)

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

l_json_t *l_query_to_json_object(l_query_t *query);
l_json_t *l_query_to_json_array(l_query_t *query);

#endif /* HAS_SQLITE3 */

/******************************************************************************
** redis.c
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

/*****************************************************************************/
#endif /* L_LHTTPD_H */
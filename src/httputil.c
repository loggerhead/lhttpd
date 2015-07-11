#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "httputil.h"


static void _http_init(l_client_t *client)
{
    bzero(&client->req, sizeof(client->req));
    http_parser_init(&client->parser, HTTP_REQUEST);
    client->req.content_length = UNINIT;
    client->parser.data = client;
}

void l_client_reset(l_client_t *client)
{
    L_FREE(client->req.url);
    L_FREE(client->req.body);
    l_free_headers(client->req.headers);

    _http_init(client);
}

l_client_t *l_create_client(l_server_t *server)
{
    l_client_t *client = l_calloc(1, sizeof(*client));

    if (uv_tcp_init(&server->loop, &client->handle)) {
        L_FREE(client);
        client = NULL;
    } else {
        _http_init(client);
        client->server = server;
        client->handle.data = client;
    }

    return client;
}

static void _on_connection_close(uv_handle_t *client_handle)
{
    l_client_t *client = client_handle->data;
    l_client_reset(client);
    L_FREE(client);
}

void l_close_connection(l_client_t *client)
{
    uv_close((uv_handle_t *) &client->handle, _on_connection_close);
}


const char *l_status_code_str(int status_code)
{
    static char *_status_codes[MAX_STATUS_CODE_NUM] = {0};
    if (_status_codes[0] == NULL) {
        _status_codes[0] = "";
#define XX(code, description) _status_codes[code] = description;
        HTTP_STATUS_CODE_MAP(XX)
#undef XX
    }
    return _status_codes[status_code];
}

l_http_response_t l_create_response()
{
    l_http_response_t response = {
        .status_code = 200,
        .headers = NULL,
        .body = NULL,
        .body_len = 0,
        .need_free = FALSE,
    };

    return response;
}

l_http_response_t l_create_redirect_response(const char *url)
{
    l_http_response_t response = l_create_response();
    response.status_code = 301;
    L_ADD_HEADER(response.headers, "Location", url);
    return response;
}

/*
 * Create response from static file.
 * NOTE: filepath can't containing "..", otherwise, 404 will be returned.
 * TODO: 1. complete headers, include: ETags, expires.
 *       2. check headers and return 304 if need(maybe need pass `args` into function)
 */
l_http_response_t l_create_response_by_file(const char *filepath)
{
    l_http_response_t response = l_create_response();

    l_buf_t tmp = {
        .data = NULL,
        .len = 0,
    };

    // prevent access file which path containing ".."
    if (strstr(filepath, "..")) {
        response.status_code = 404;
    } else {
        tmp = l_read_file(filepath);
        L_ADD_HEADER(response.headers, "Content-Type", l_get_mimetype(filepath));
    }

    response.body = tmp.data;
    response.body_len = tmp.len;
    response.need_free = TRUE;

    return response;
}

l_buf_t l_generate_response_data(l_client_t *client, l_http_response_t response)
{
    int status_code = response.status_code;
    l_hitem_t *headers = response.headers;

    int http_major = client->parser.http_major;
    int http_minor = client->parser.http_minor;
    http_major = http_major ? http_major : 1;
    http_minor = http_minor >= 0 ? http_minor : 0;

    // concat Status-Line
    char *rdata;
    char *tmp = l_mprintf("HTTP/%d.%d %d %s\r\nserver: %s",
                          http_major, http_minor,
                          status_code, l_status_code_str(status_code),
                          APP_NAME);

#define _RELAY(tmp, rdata) do { L_FREE(tmp); tmp = rdata; } while (0)

    // concat headers
    L_HITER(headers, h) {
        rdata = l_mprintf("%s\r\n%s: %s", tmp, h->key, h->value);
        _RELAY(tmp, rdata);
    }

    if (!l_get_header(headers, "Content-Type") && !l_get_header(headers, "content-type")) {
        rdata = l_mprintf("%s\r\nContent-Type: text/plain; charset=us-ascii", tmp);
        _RELAY(tmp, rdata);
    }
#undef _RELAY

    // concat body
    const char *body = response.body;
    size_t body_len  = response.body_len;
    if (!body)
        body = "";
    if (!body_len)
        body_len = strlen(body);

    rdata = l_mprintf("%s\r\nContent-Length: %d\r\n\r\n",
                      tmp, body_len);
    L_FREE(tmp);

    size_t rdata_len = strlen(rdata);
    if (body_len) {
        rdata = l_realloc(rdata, rdata_len + body_len);
        memcpy(rdata + rdata_len, body, body_len);
    }
    rdata_len += body_len;

    l_buf_t r = { rdata, rdata_len };
    return r;
}


typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

static void _free_write_req(uv_write_t *req, int status)
{
    if (status < 0) {
        l_warn("send bytes error: %s", uv_strerror(status));
    }
    write_req_t *wr = (write_req_t *) req;
    free(wr->buf.base);
    free(wr);
}

const char *l_send_bytes(l_client_t *client, const char *bytes, size_t len)
{
    write_req_t *req = l_malloc(sizeof(*req));
    req->buf = uv_buf_init((char *) l_malloc(len), len);
    memcpy(req->buf.base, bytes, len);

    uv_write((uv_write_t *) req, (uv_stream_t *) &client->handle, &req->buf, 1, _free_write_req);

    return "";
}

const char *l_send_response(l_client_t *client, l_http_response_t response)
{
    l_buf_t r = l_generate_response_data(client, response);
    const char *errmsg = l_send_bytes(client, r.data, r.len);

    L_FREE(r.data);
    if (response.need_free)
        L_FREE(response.body);

    return errmsg;
}

const char *l_send_code(l_client_t *client, int status_code)
{
    l_http_response_t response = { status_code, NULL, NULL };
    return l_send_response(client, response);
}

const char *l_send_body(l_client_t *client, const char *body)
{
    l_http_response_t response = { 200, NULL, body };
    return l_send_response(client, response);
}


l_hitem_t *l_add_header(l_hitem_t *headers, const char *field, const char *value)
{
    return l_hput(headers, field, value);
}

char *l_get_header(l_hitem_t *headers, const char *field)
{
    return l_hget(headers, field);
}

static void _free_header(l_hitem_t *header)
{
    L_FREE(header->key);
    L_FREE(header->value);
}

void l_free_headers(l_hitem_t *headers)
{
    l_hfree(headers, _free_header);
}

void l_print_headers(l_hitem_t *headers)
{
    l_hitem_t *header;

    for(header=headers; header; header=header->hh.next)
        l_log("%s: %s", header->key, header->value);
}

l_bool_t l_is_implemented_http_method(l_client_t *client)
{
    static l_bool_t _implemented[MAX_HTTP_METHOD_NUM] = { FALSE };
    if (_implemented[MAX_HTTP_METHOD_NUM-1] == FALSE) {
        _implemented[MAX_HTTP_METHOD_NUM-1] = TRUE;
#define XX(method) _implemented[HTTP_ ## method] = TRUE;
        IMPLEMENTED_HTTP_METHOD_MAP(XX)
#undef XX
    }

    return _implemented[client->req.method];
}

l_bool_t l_is_http_get(l_client_t *client)
{
    return client->req.method == HTTP_GET;
}

l_bool_t l_is_http_post(l_client_t *client)
{
    return client->req.method == HTTP_POST;
}

l_bool_t l_is_http_head(l_client_t *client)
{
    return client->req.method == HTTP_HEAD;
}

l_bool_t l_is_http_delete(l_client_t *client)
{
    return client->req.method == HTTP_DELETE;
}

l_bool_t l_is_http_put(l_client_t *client)
{
    return client->req.method == HTTP_PUT;
}

const char *l_get_mimetype(const char *filepath)
{
    static l_hitem_t *mimetypes = NULL;

    if (!mimetypes) {
        static const char *knownfiles[] = {
            "/etc/mime.types",
            "/etc/httpd/mime.types",                    // Mac OS X
            "/etc/httpd/conf/mime.types",               // Apache
            "/etc/apache/mime.types",                   // Apache 1
            "/etc/apache2/mime.types",                  // Apache 2
            "/usr/local/etc/httpd/conf/mime.types",
            "/usr/local/lib/netscape/mime.types",
            "/usr/local/etc/httpd/conf/mime.types",     // Apache 1.2
            "/usr/local/etc/mime.types",                // Apache 1.3
        };

        // open mimetype file
        size_t knownlen = sizeof(knownfiles) / sizeof(const char *);
        FILE *fp = NULL;
        for (int i = 0; i < knownlen; i++) {
            if (l_is_file_exist(knownfiles[i])) {
                fp = fopen(knownfiles[i], "r");
                break;
            }
        }
        assert(fp);

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
            char *mimetype = NULL;
            char *filetype = NULL;
            size_t linelen = strlen(line);
            int i = 0;

            while (i < linelen && line[i] != '#') {
                while (i < linelen && isspace(line[i]))
                    i++;

                if (i == linelen || line[i] == '#')
                    break;

                if (!mimetype)
                    mimetype = line + i;
                else
                    filetype = line + i;

                while (i < linelen && !isspace(line[i]) && line[i] != '#')
                    i++;
                line[i++] = '\0';

                if (filetype)
                    L_HPUT(mimetypes, strdup(filetype), strdup(mimetype));
            }
        }

        fclose(fp);
    }

    int i;
    char suffix[256];
    const char *tmp = l_get_suffix(filepath);
    for (i = 0; tmp[i]; i++)
        suffix[i] = tolower(tmp[i]);
    suffix[i] = '\0';

    const char *mimetype = l_hget(mimetypes, suffix);
    return mimetype ? mimetype : "application/octet-stream";
}
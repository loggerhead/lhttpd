#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "httputil.h"


static void _free_write_req(uv_write_t *req, int status)
{
    if (status < 0)
        LOG_ERROR(ERR_UV_WRITE);

    write_req_t *wr = (write_req_t *) req;
    free(wr->buf.base);
    free(wr);
}

static void _free_header(l_hitem_t *header)
{
    L_FREE(header->key);
    L_FREE(header->value);
}

static int _free_request(l_http_request_t *request)
{
    L_FREE(request->hook);
    L_FREE(request->url);
    L_FREE(request->body);
    l_free_headers(request->headers);
    request->hook = NULL;
    request->url = NULL;
    request->body = NULL;
    request->headers = NULL;
    return 0;
}

static int _free_response(l_http_response_t *response)
{
    l_free_headers(response->headers);
    L_FREE(response->body);
    response->headers = NULL;
    response->body = NULL;
    return 0;
}

static void _del_header(l_hitem_t *headers, const char *field)
{
    l_hitem_t *item = NULL;
    HASH_FIND_STR(headers, field, item);

    if (item) {
        HASH_DEL(headers, item);
        _free_header(item);
        L_FREE(item);
    }
}

static void _reset_http_parser(l_client_t *client)
{
    bzero(&client->request, sizeof(client->request));
    http_parser_init(&client->parser, HTTP_REQUEST);
    client->parser.data = client;
}

static void _on_connection_close(uv_handle_t *client_handle)
{
    l_client_t *client = client_handle->data;
    l_reset_connection(client);
    L_FREE(client);
}


l_client_t *l_create_connection(l_server_t *server)
{
    l_client_t *client = l_calloc(1, sizeof(*client));

    if (uv_tcp_init(&server->loop, &client->handle)) {
        L_FREE(client);
        client = NULL;
    } else {
        _reset_http_parser(client);
        client->response = l_create_response();
        client->server = server;
        client->handle.data = client;
        client->close_connection = FALSE;
    }

    return client;
}

void l_reset_connection(l_client_t *client)
{
    _free_request(&client->request);
    _free_response(&client->response);
    _reset_http_parser(client);
    client->response = l_create_response();
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
        .callback = _free_response,
    };

    return response;
}

l_http_response_t l_create_redirect_response(const char *url)
{
    l_http_response_t response = l_create_response();
    response.status_code = 301;
    L_PUT_HEADER(response.headers, "location", url);
    return response;
}

/*
 * Create response from static file.
 * NOTE: filepath can't containing "..", otherwise, 404 will be returned.
 *
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
        L_PUT_HEADER(response.headers, "Content-Type", l_get_mimetype(filepath));
    }

    l_set_response_body(&response, tmp.data, tmp.len);
    L_FREE(tmp.data);
    return response;
}

void l_set_response_body(l_http_response_t *response, const char *body, size_t body_len)
{
    L_FREE(response->body);
    char *buf = l_malloc(body_len);
    memcpy(buf, body, body_len);

    response->body = buf;
    response->body_len = body_len;
}


int l_send_bytes(l_client_t *client, const char *bytes, size_t len)
{
    write_req_t *req = l_malloc(sizeof(*req));
    req->buf = uv_buf_init((char *) l_malloc(len), len);
    memcpy(req->buf.base, bytes, len);

    int err = uv_write((uv_write_t *) req,
                       (uv_stream_t *) &client->handle,
                       &req->buf, 1,
                       _free_write_req);

    return err ? ERR_UV_WRITE : 0;
}

int l_send_response_line(l_client_t *client, int http_minor, int status_code)
{
    char *tmp = l_mprintf("HTTP/1.%d %d %s" CRLF,
                          http_minor, status_code, l_status_code_str(status_code));
    int err = l_send_bytes(client, tmp, strlen(tmp));
    L_FREE(tmp);
    return err;
}

int l_send_header(l_client_t *client, const char *field, const char *value)
{
    const char *tmp = l_mprintf("%s: %s" CRLF, field, value);
    int err = l_send_bytes(client, tmp, strlen(tmp));
    L_FREE(tmp);
    return err;
}

int l_send_blankline(l_client_t *client)
{
    return l_send_bytes(client, CRLF, strlen(CRLF));
}

int l_send_response(l_client_t *client, l_http_response_t *response)
{
    int err = 0;
    int status_code = response->status_code ? response->status_code : 200;
    int status_code_class = status_code / 100;
    l_bool_t should_send_body = response->body_len && l_is_str(response->body);

    // send error message for the 4xx and 5xx responses
    if (status_code_class == 4 || status_code_class == 5) {
        const char *errmsg = l_mprintf(HTTP_ERRMSG_FMT, status_code, l_status_code_str(status_code));
        l_set_response_body(response, errmsg, strlen(errmsg));
        L_FREE(errmsg);
        should_send_body = TRUE;
    // the 1xx, 204, and 304 responses MUST NOT include a body (see rfc2616 section-4.4)
    } else if (status_code_class == 1 || status_code == 204 || status_code == 304) {
        should_send_body = FALSE;
    }

    _(l_send_response_line(client, client->parser.http_minor, status_code));

    if (!l_get_header(response->headers, "content-type"))
        L_PUT_HEADER(response->headers, "content-type", "text/html");

    if (should_send_body) {
        char *tmp = l_mprintf("content-length: %d" CRLF, response->body_len);
        _(l_send_bytes(client, tmp, strlen(tmp)));
        L_FREE(tmp);
    // TODO: handle more HTTP response code
    } else {
        char *tmp = "content-length: 0" CRLF;
        _(l_send_bytes(client, tmp, strlen(tmp)));
    }

    L_HITER(response->headers, h) {
        _(l_send_header(client, h->key, h->value));
    }

    _(l_send_blankline(client));

    if (should_send_body)
        _(l_send_bytes(client, response->body, response->body_len));

END:;
    return err;
}

int l_send_code(l_client_t *client, int status_code)
{
    l_http_response_t response = l_create_response();
    response.status_code = status_code;
    return l_send_response(client, &response);
}

int l_send_body(l_client_t *client, const char *body)
{
    l_http_response_t response = l_create_response();
    if (l_is_str(body))
        l_set_response_body(&response, body, strlen(body));
    return l_send_response(client, &response);
}


/* equivalent to `headers[field.lower()] = value`
 * NOTE: `l_put_header` will alloc memory for `field` and `value`
 */
l_hitem_t *l_put_header(l_hitem_t *headers, const char *field, const char *value)
{
    char *key = strdup(field);
    l_lowercase(key);

    _del_header(headers, key);

    return l_hput(headers, key, strdup(value));
}

// equivalent to `return headers[field.lower()]`
char *l_get_header(l_hitem_t *headers, const char *field)
{
    char *key = strdup(field);
    l_lowercase(key);

    char *value = l_hget(headers, key);

    L_FREE(key);
    return value;
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

    return _implemented[client->request.method];
}

l_bool_t l_is_http_get(l_client_t *client)
{
    return client->request.method == HTTP_GET;
}

l_bool_t l_is_http_post(l_client_t *client)
{
    return client->request.method == HTTP_POST;
}

l_bool_t l_is_http_head(l_client_t *client)
{
    return client->request.method == HTTP_HEAD;
}

l_bool_t l_is_http_delete(l_client_t *client)
{
    return client->request.method == HTTP_DELETE;
}

l_bool_t l_is_http_put(l_client_t *client)
{
    return client->request.method == HTTP_PUT;
}


/* return path part of url. For example, `www.test.com/foo` will return `/foo`
 * NOTE: need free
 */
const char *l_get_url_path(l_client_t *client)
{
    struct http_parser_url u = client->request.parsed_url;
    const char *url_path = client->request.url + u.field_data[UF_PATH].off;

    if (u.field_set & (1 << UF_PATH))
        return strndup(url_path, u.field_data[UF_PATH].len);
    return strdup("/");
}

const char *l_get_mimetype(const char *filepath)
{
    static l_hitem_t *mimetypes = NULL;

    // read mimetypes file into hash table
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

        // open mimetypes file
        size_t knownlen = sizeof(knownfiles) / sizeof(const char *);
        FILE *fp = NULL;
        for (int i = 0; i < knownlen; i++) {
            if (l_is_file_exist(knownfiles[i])) {
                fp = fopen(knownfiles[i], "r");
                break;
            }
        }

        if (!fp) {
            LOG_ERROR(ERR_MIMETYPES);
            goto END;
        }

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

END:;
    const char *suffix = strdup(l_get_suffix(filepath));
    const char *mimetype = l_hget(mimetypes, suffix);
    L_FREE(suffix);
    return mimetype ? mimetype : "application/octet-stream";
}
#include "httpclient.h"
#include "httputil.h"
#include "util.h"

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
    l_http_response_t response = {200, NULL, NULL};
    return response;
}

char *l_generate_response_str(l_client_t *client, l_http_response_t response)
{
    int status_code = response.status_code;
    l_hitem_t *headers = response.headers;
    const char *body = response.body;

    int http_major = client->parser.http_major;
    int http_minor = client->parser.http_minor;
    http_major = http_major ? http_major : 1;
    http_minor = http_minor >= 0 ? http_minor : 0;

    char *tmp = l_mprintf("HTTP/%d.%d %d %s\r\n"
                          "server: %d/%s",
                          http_major, http_minor,
                          status_code, l_status_code_str(status_code),
                          APP_NAME, APP_VERSION);
    char *rstr;

#define _RELAY(tmp, rstr) do { L_FREE(tmp); tmp = rstr; } while (0)
    L_HITER(headers, h) {
        rstr = l_mprintf("%s\r\n%s: %s", tmp, h->key, h->value);
        _RELAY(tmp, rstr);
    }

    if (!l_get_header(headers, "Content-Type") && !l_get_header(headers, "content-type")) {
        rstr = l_mprintf("%s\r\n%s: %s", tmp,
                         "Content-Type", "text/plain; charset=us-ascii");
        _RELAY(tmp, rstr);
    }

    if (!body)
        body = "";
    rstr = l_mprintf("%s\r\n%s: %d\r\n" "\r\n%s", tmp,
                     "Content-Length", strlen(body),
                     body);
    L_FREE(tmp);
#undef _RELAY

    return rstr;
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
    char *rstr = l_generate_response_str(client, response);
    const char *errmsg = l_send_bytes(client, rstr, strlen(rstr));
    L_FREE(rstr);

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
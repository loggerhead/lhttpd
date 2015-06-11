#include "../config.h"
#include "httpclient.h"
#include "httputil.h"
#include "util.h"

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;


void l_free_request(l_client_t *client)
{
    L_FREE(client->url);
    L_FREE(client->body);
    l_free_headers(client->headers);
}

void l_reset_client(l_client_t *client)
{
    l_free_request(client);

    http_parser_init(&client->parser, HTTP_REQUEST);

    client->is_message_complete = 0;

    client->url = NULL;
    client->headers = NULL;
    client->body = NULL;
    client->content_length = UNINIT;
    client->readed_len = 0;
}

l_client_t *l_get_client_instance(l_server_t *server)
{
    l_client_t *client = l_calloc(1, sizeof(l_client_t));

    if (uv_tcp_init(&server->loop, &client->handle) == 0) {
        http_parser_init(&client->parser, HTTP_REQUEST);
        // set `xxx.data` as a hook
        client->handle.data = client;
        client->parser.data = client;
        client->server = server;
    } else {
        L_FREE(client);
        client = NULL;
    }

    return client;
}


static l_hitem_t *_hashtbl;

const char *l_status_code(const char *code)
{
    if (!_hashtbl) {
#define XX(key, value) _hashtbl = l_hput(_hashtbl, key, value);
        HTTP_STATUS_CODE_MAP(XX)
#undef XX
    }

    return l_hget(_hashtbl, code);
}

char *l_generate_response(l_client_t *client, const char *status_code,
                          l_hitem_t *headers, const char *body)
{
    int http_major = client->parser.http_major;
    int http_minor = client->parser.http_minor;
    http_major = http_major ? http_major : 1;
    http_minor = http_minor >= 0 ? http_minor : 0;

    char *tmp = l_mprintf("HTTP/%d.%d %s %s\r\n"
                          "server: %s/%s",
                          http_major, http_minor,
                          status_code, l_status_code(status_code),
                          APP_NAME, APP_VERSION);
    char *response;

    l_hitem_t *h;
    L_HITER(headers, h) {
        response = l_mprintf("%s\r\n%s: %s", tmp, h->key, h->value);
        L_FREE(tmp);
        tmp = response;
    }

    if (body && *body) {
        char *content_type = l_get_header(headers, "Content-Type");
        if (!content_type)
            content_type = "Content-Type: text/plain; charset=us-ascii";

        response = l_mprintf("%s\r\n"
                             "%s\r\n"
                             "Content-Length: %d\r\n"
                             "\r\n%s",
                             tmp, content_type, strlen(body), body);
    } else {
        response = l_mprintf("%s\r\n\r\n", tmp);
    }

    L_FREE(tmp);
    return response;
}


static void free_write_req(uv_write_t *req, int status)
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

    uv_write((uv_write_t *) req, (uv_stream_t *) &client->handle, &req->buf, 1, free_write_req);

    return "";
}

const char *l_send_response(l_client_t *client, const char *status_code,
                            l_hitem_t *headers, const char *body)
{
    headers = l_hput(headers, "Connection", "keep-alive");

    char *response = l_generate_response(client, status_code, headers, body);
    const char *errmsg = l_send_bytes(client, response, strlen(response));
    L_FREE(response);

    return errmsg;
}

const char *l_send_code(l_client_t *client, const char *status_code)
{
    return l_send_response(client, status_code, NULL, NULL);
}

const char *l_send_body(l_client_t *client, const char *body)
{
    return l_send_response(client, "200", NULL, body);
}
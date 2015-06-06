#include "../config.h"
#include "httpclient.h"
#include "httputil.h"
#include "util.h"

char *l_get_ip(client_t *client, char ip[17])
{
    struct sockaddr addr;
    int namelen = sizeof(addr);

    uv_tcp_getsockname(&client->handle, &addr, &namelen);
    uv_ip4_name((const struct sockaddr_in *)&addr, ip, 17);

    return ip;
}

void l_free_request(client_t *client)
{
    L_FREE(client->url);
    l_free_headers(client->headers);
    L_FREE(client->body);
}

void l_reset_client(client_t *client)
{
    // TODO: remove comment
    // l_free_request(client);

    http_parser_init(&client->parser, HTTP_REQUEST);

    client->is_message_complete = 0;

    client->url = NULL;
    client->headers = NULL;
    client->body = NULL;
    client->content_length = UNINIT;
    client->readed_len = 0;
}

client_t *l_get_client_instance(server_t *server)
{
    client_t *client = l_calloc(1, sizeof(client_t));

    UV_CHECK(uv_tcp_init(&server->loop, &client->handle));
    // set `xxx.data` as a hook
    client->handle.data = client;
    client->parser.data = client;
    client->server = server;

    return client;
}


static hitem_t *_hashtbl;

const char *l_status_code(const char *code)
{
    if (!_hashtbl) {
#define XX(key, value) _hashtbl = l_hput(_hashtbl, key, value);
        HTTP_STATUS_CODE_MAP(XX)
#undef XX
    }

    return l_hget(_hashtbl, code);
}

char *l_generate_response(client_t *client, const char *status_code,
                          hitem_t *headers, const char *body)
{
    int http_major = client->parser.http_major;
    int http_minor = client->parser.http_minor;
    http_major = http_major ? http_major : 1;
    http_minor = http_minor >= 0 ? http_minor : 0;

    char *tmp = l_mprintf("HTTP/%d.%d %s %s\r\n"
                          "server: %s/%s",
                          http_minor, http_minor,
                          status_code, l_status_code(status_code),
                          APP_NAME, APP_VERSION);
    char *response;

    hitem_t *h;
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
        L_FREE(tmp);
    } else {
        response = l_mprintf("%s\r\n\r\n", tmp);
        L_FREE(tmp);
    }

    return response;
}


const char *l_send_bytes(client_t *client, const char *bytes, size_t len)
{
    uv_write_t req;
    uv_buf_t out = {
        .base = (char *)bytes,
        .len  = len
    };

    uv_write(&req, (uv_stream_t *) &client->handle, &out, 1, NULL);

    return "";
}

const char *l_send_response(client_t *client, const char *status_code,
                            hitem_t *headers, const char *body)
{
    headers = l_hput(headers, "Connection", "keep-alive");

    char *response = l_generate_response(client, status_code, headers, body);
    const char *errmsg = l_send_bytes(client, response, strlen(response));
    // TODO: free response after send
    // L_FREE(response);

    return errmsg;
}

const char *l_send_code(client_t *client, const char *status_code)
{
    return l_send_response(client, status_code, NULL, NULL);
}

const char *l_send_body(client_t *client, const char *body)
{
    return l_send_response(client, "200", NULL, body);
}
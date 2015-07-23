#include <stdlib.h>
#include "webrouter.h"
#include "httpserver.h"
#include "httputil.h"
#include "util.h"

static void _alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    *buf = uv_buf_init(l_malloc(suggested_size), suggested_size);
}

// HTTP parse
static int _on_url(http_parser *parser, const char *at, size_t len)
{
    l_client_t *client = parser->data;

    client->req.url = strndup(at, len);
    return 0;
}

static int _on_header_field(http_parser *parser, const char *at, size_t len)
{
    l_client_t *client = parser->data;

    const char *field = strndup(at, len);

    client->req.headers = l_add_header(client->req.headers, field, NULL);

    if (!strcasecmp("content-length", field))
        client->req.content_length = WAIT_FOR_VALUE;

    return 0;
}

static int _on_header_value(http_parser *parser, const char *at, size_t len)
{
    l_client_t *client = parser->data;

    const char *value = strndup(at, len);

    // TODO: raise 413 error when content_length > 102400
    if (client->req.content_length == WAIT_FOR_VALUE)
        client->req.content_length = atoi(value);

    for (l_hitem_t *h = client->req.headers; h; h = h->hh.next) {
        if (h->value == NULL) {
            client->req.headers = l_add_header(client->req.headers, h->key, value);
            break;
        }
    }

    return 0;
}

static int _on_body(http_parser *parser, const char *at, size_t len)
{
    l_client_t *client = parser->data;

    if (!client->req.body) {
        client->req.body = strndup(at, len);
    } else {
        char *tmp = l_realloc(client->req.body, client->req.body_nread + len);

        if (tmp) {
            stpncpy(tmp + client->req.body_nread, at, len);
            client->req.body = tmp;
        } else {
            l_warn("%s: can't alloc enough memory", __func__);
            return HPE_UNKNOWN;
        }
    }

    client->req.body_nread += len;

    return 0;
}

static int _on_message_complete(http_parser *parser)
{
    l_client_t *client = parser->data;
    client->req.method = client->parser.method;

    if (client->req.content_length == UNINIT)
        client->req.content_length = 0;

    client->req.is_finished = TRUE;

    return 0;
}

static http_parser_settings *_get_http_parser_settings()
{
    static http_parser_settings *parser_settings = NULL;

    if (parser_settings == NULL) {
        parser_settings = l_calloc(1, sizeof(http_parser_settings));

        parser_settings->on_url = _on_url;
        parser_settings->on_header_field = _on_header_field;
        parser_settings->on_header_value = _on_header_value;
        parser_settings->on_body = _on_body;
        parser_settings->on_message_complete = _on_message_complete;
    }

    return parser_settings;
}

static int _do_http_parse(l_client_t *client, const char *at, size_t len)
{
    size_t nparsed = http_parser_execute(&client->parser, _get_http_parser_settings(), at, len);

    if (nparsed != len) {
        l_warn("%s: %s", __func__, http_errno_description(HTTP_PARSER_ERRNO(&client->parser)));
        return -1;
    }
    return 0;
}

static const char *_server_on_request(l_client_t *client)
{
    if (!l_is_implemented_http_method(client)) {
        l_send_code(client, 405);
        return "Not implement http method";
    }

    l_route_match_t match = l_match_route(client->req.url, client->req.method);

    l_http_response_t response = l_create_response();
    if (match.callback) {
        response = match.callback(client, match.args);
    } else {
        response.status_code = 404;
    }

    l_free_match(match);
    return l_send_response(client, response);
}

static const char *_server_on_data(l_client_t *client, const char *buf, ssize_t nread)
{
    const char *errmsg = NULL;

    l_server_t *server = client->server;

    if (_do_http_parse(client, buf, nread)) {
        errmsg = "parse HTTP request failed";
        l_client_reset(client);
    } else if (client->req.is_finished) {
        if (server->on_request_cb)
            errmsg = server->on_request_cb(client);

        l_client_reset(client);
    }

    return errmsg;
}

static void _server_on_read(uv_stream_t *client_handle, ssize_t nread, const uv_buf_t *buf)
{
    l_client_t *client = client_handle->data;
    l_server_t *server = client->server;
    const char *errmsg = NULL;

    // nread == 0 indicating that at this point there is nothing to be read.
    if (nread > 0) {
        if (server->on_data_cb)
            errmsg = server->on_data_cb(client, buf->base, nread);
    } else if (nread < 0) {
        if (nread != UV_EOF)
            errmsg = "incorrect read";
    }

    L_FREE(buf->base);

    // close connection if any error or other end closed
    if (l_has_str(errmsg)) {
        l_warn("%s: %s", __func__, errmsg);
        l_close_connection(client);
    } else if (nread == UV_EOF) {
        l_close_connection(client);
    }
}


static void _server_on_connect(uv_stream_t *server_handle, int status)
{
    l_client_t *client = l_create_client(server_handle->data);

    if (client == NULL) {
        l_warn("%s: uv_tcp_init failed when connect", __func__);
        return;
    }

    // accept connection, and `client->handle` is the client end
    if (uv_accept(server_handle, (uv_stream_t *) &client->handle) == 0) {
        uv_read_start((uv_stream_t *) &client->handle, _alloc_buffer, _server_on_read);
    } else {
        l_close_connection(client);
    }
}

l_server_t *l_create_server()
{
    l_server_t *server = l_calloc(1, sizeof(*server));
    server->ip = "0.0.0.0";
    server->port = DEFAULT_PORT;
    server->handle.data = server;

    server->on_data_cb = _server_on_data;
    server->on_request_cb = _server_on_request;

    return server;
}

void l_set_ip_port(l_server_t *server, const char *ip, int port)
{
    if (ip)
        server->ip = ip;
    if (port)
        server->port = port;
}

static l_server_t *_server;

static void _free_server()
{
    L_FREE(_get_http_parser_settings());
    l_free_routes();
    L_FREE(_server);
}

void l_start_server(l_server_t *server)
{
    _server = server;
    atexit(_free_server);

    struct sockaddr_in addr;

    UV_CHECK(uv_loop_init(&server->loop));
    UV_CHECK(uv_tcp_init(&server->loop, &server->handle));
    UV_CHECK(uv_ip4_addr(server->ip, server->port, &addr));
    UV_CHECK(uv_tcp_bind(&server->handle, (const struct sockaddr *) &addr, 0));
    UV_CHECK(uv_listen((uv_stream_t *) &server->handle, 128, _server_on_connect));

    uv_run(&server->loop, UV_RUN_DEFAULT);

    uv_loop_close(&server->loop);
}
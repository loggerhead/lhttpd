#include "httpclient.h"
#include "httpserver.h"
#include "httputil.h"
#include "util.h"

static void l_server_on_close(uv_handle_t *handle);


static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    *buf = uv_buf_init(l_malloc(suggested_size), suggested_size);
}

// HTTP parse
static int on_url(http_parser *parser, const char *at, size_t len)
{
    client_t *client = parser->data;

    client->url = strndup(at, len);
    return 0;
}

static int on_header_field(http_parser *parser, const char *at, size_t len)
{
    client_t *client = parser->data;

    const char *field = strndup(at, len);

    client->headers = l_add_header(client->headers, field, NULL);

    if (!strcasecmp("content-length", field))
        client->content_length = WAIT_FOR_VALUE;

    return 0;
}

static int on_header_value(http_parser *parser, const char *at, size_t len)
{
    client_t *client = parser->data;

    const char *value = strndup(at, len);

    if (client->content_length == WAIT_FOR_VALUE)
        client->content_length = atoi(value);

    for (hitem_t *h = client->headers; h; h = h->hh.next) {
        if (h->value == NULL) {
            client->headers = l_add_header(client->headers, h->key, value);
            break;
        }
    }

    return 0;
}

static int on_body(http_parser *parser, const char *at, size_t len)
{
    client_t *client = parser->data;

    if (!client->body) {
        client->body = strndup(at, len);
    } else {
        char *tmp = l_realloc(client->body, client->readed_len+len);

        if (tmp) {
            stpncpy(tmp+client->readed_len, at, len);
            client->body = tmp;
        } else {
            char ip[17];
            l_warn("Can't alloc enough memory for %s", l_get_ip(client, ip));
            return HPE_UNKNOWN;
        }
    }

    client->readed_len += len;

    return 0;
}

static int on_message_complete(http_parser *parser)
{
    client_t *client = parser->data;

    if (client->content_length == UNINIT)
        client->content_length = 0;

    client->is_message_complete = 1;
    return 0;
}

static http_parser_settings *get_http_settings()
{
    static http_parser_settings *parser_settings = NULL;

    if (!parser_settings) {
        parser_settings = l_calloc(1, sizeof(http_parser_settings));

        parser_settings->on_url = on_url;
        parser_settings->on_header_field = on_header_field;
        parser_settings->on_header_value = on_header_value;
        parser_settings->on_body = on_body;
        parser_settings->on_message_complete = on_message_complete;
    }

    return parser_settings;
}

static int do_http_parse(http_parser *parser, const char *at, size_t length)
{
    size_t nparsed = http_parser_execute(parser, get_http_settings(), at, length);

    if (nparsed != length) {
        l_warn("%s: %s", __func__, http_errno_description(HTTP_PARSER_ERRNO(parser)));
        return -1;
    }
    return 0;
}

// request data handler
static const char *l_server_on_request(client_t *client)
{
    // TODO: add route and request support
    switch(client->parser.method) {
        case HTTP_GET:
        case HTTP_POST:
            break;
        default:
            l_send_code(client, "405");
            return "Not implement http method";
    }

    return "";
}

static const char *l_server_on_data(client_t *client, const char *buf, ssize_t nread)
{
    TRACE();

    const char *errmsg = "";

    server_t *server = client->server;

    if (do_http_parse(&client->parser, buf, nread)) {
        errmsg = "parse HTTP request failed";
    } else if (client->is_message_complete) {
        // TODO: BUG when called on_request_cb;
        if (server->on_request_cb)
            errmsg = server->on_request_cb(client);

        l_reset_client(client);
    }

    return errmsg;
}

static void l_server_on_read(uv_stream_t *client_handle, ssize_t nread, const uv_buf_t *buf)
{
    client_t *client = client_handle->data;
    server_t *server = client->server;
    const char *errmsg = "";

    if (nread > 0) {
        if (server->on_data_cb)
            errmsg = server->on_data_cb(client, buf->base, nread);
    } else if (nread < 0) {
        errmsg = (nread == UV_EOF)? "client closed" : "incorrect read";
    }

    L_FREE(buf->base);

    if (l_has_error(errmsg)) {
        l_warn("%s: %s", __func__, errmsg);
        uv_close((uv_handle_t *) &client->handle, l_server_on_close);
    }
}


static void l_server_on_connect(uv_stream_t *server_handle, int status)
{
    TRACE();

    UV_CHECK(status);

    // init client
    client_t *client = l_get_client_instance(server_handle->data);
    server_t *server = server_handle->data;

    // accept connection, and `client->handle` is the client end
    UV_CHECK(uv_accept(server_handle, (uv_stream_t *) &client->handle));

    // read data from connection and invoke `server->on_read_cb`
    if (server->on_read_cb)
        uv_read_start((uv_stream_t *) &client->handle, alloc_buffer, server->on_read_cb);
}

static void l_server_on_close(uv_handle_t *client_handle)
{
    TRACE();

    client_t *client = client_handle->data;
    l_reset_client(client);
    L_FREE(client);
}


server_t *l_get_server_instance()
{
    server_t *server = l_calloc(1, sizeof(*server));
    server->ip = "0.0.0.0";
    server->port = DEFAULT_PORT;
    server->handle.data = server;

    server->on_read_cb = l_server_on_read;
    server->on_data_cb = l_server_on_data;
    server->on_request_cb = l_server_on_request;

    return server;
}

void l_set_ip_port(server_t *server, const char *ip, int port)
{
    if (ip)
        server->ip = ip;
    if (port)
        server->port = port;
}

void l_start_server(server_t *server)
{
    struct sockaddr_in addr;

    UV_CHECK(uv_loop_init(&server->loop));
    UV_CHECK(uv_tcp_init(&server->loop, &server->handle));
    UV_CHECK(uv_ip4_addr(server->ip, server->port, &addr));
    UV_CHECK(uv_tcp_bind(&server->handle, (const struct sockaddr *) &addr, 0));
    UV_CHECK(uv_listen((uv_stream_t *) &server->handle, 128, l_server_on_connect));

    uv_run(&server->loop, UV_RUN_DEFAULT);
    uv_loop_close(&server->loop);

    L_FREE(server);
}
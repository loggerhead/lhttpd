/* TODO: finish chunk
 * When on_chunk_header is called, the current chunk length is stored in parser->content_length.
 *   `static int _on_chunk_header(http_parser *parser)`
 *   `static int _on_chunk_complete(http_parser *parser)`
 */

#include <stdlib.h>
#include "server.h"
#include "webrouter.h"
#include "httputil.h"
#include "util.h"


static void _alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    *buf = uv_buf_init(l_malloc(suggested_size), suggested_size);
}


static int _on_url(http_parser *parser, const char *at, size_t len)
{
    int err = 0;
    l_client_t *client = parser->data;
    client->request.url = strndup(at, len);

    _(http_parser_parse_url(at, len, 0, &client->request.parsed_url));
    struct http_parser_url u = client->request.parsed_url;

    int schema_len = u.field_data[UF_SCHEMA].len;
    if (schema_len) {
        _(schema_len != 4);
        _(strncmp("http", at + u.field_data[UF_SCHEMA].off, 4));
    }

END:;
    return err;
}

static int _on_header_field(http_parser *parser, const char *at, size_t len)
{
    l_client_t *client = parser->data;
    client->request.hook = strndup(at, len);
    return 0;
}

// NOTE: not support multiple values for one field
static int _on_header_value(http_parser *parser, const char *at, size_t len)
{
    int err = 0;
    l_client_t *client = parser->data;
    l_server_t *server = client->server;
    const char *field = client->request.hook;
    const char *value = strndup(at, len);

    if (l_is_strcaseeq("content-length", field)) {
        long content_length = strtol(value, NULL, 10);
        client->request.content_length = content_length;

        if (content_length > server->max_content_length) {
            client->response.status_code = 413;
            err = 1;
        }
    } else if (l_is_strcaseeq("Expect", field)) {
        if (l_is_strcaseeq("100-continue", value))
            client->response.status_code = 100;
    } else {
        L_PUT_HEADER(client->request.headers, field, value);
    }

    L_FREE(field);
    L_FREE(value);
    client->request.hook = NULL;
    return err;
}

static int _on_headers_complete(http_parser *parser)
{
    l_client_t *client = parser->data;
    client->request.method = parser->method;
    int http_major = parser->http_major;
    int http_minor = parser->http_minor;

    // check HTTP version
    if (http_major != 1) {
        client->response.status_code = 505;
        return 1;
    } else if (http_minor > 0) {
        client->close_connection = FALSE;
    }

    const char *date = l_gmtime();
    L_PUT_HEADER(client->response.headers, "server", APP_NAME);
    L_PUT_HEADER(client->response.headers, "date", date);
    L_FREE(date);

    return 0;
}

static int _on_body(http_parser *parser, const char *at, size_t len)
{
    l_client_t *client = parser->data;

    if (!client->request.body) {
        client->request.body = strndup(at, len);
    } else {
        char *tmp = l_realloc(client->request.body, client->request.body_nread + len);

        if (tmp) {
            stpncpy(tmp + client->request.body_nread, at, len);
            client->request.body = tmp;
        } else {
            LOG_ERROR(ERR_MEMORY_ALLOC);
            client->response.status_code = 500;
            return 1;
        }
    }

    client->request.body_nread += len;

    return client->request.body_nread > client->request.content_length;
}

static int _on_message_complete(http_parser *parser)
{
    l_client_t *client = parser->data;
    client->request.is_finished = TRUE;

    if (client->request.content_length == 0)
        L_PUT_HEADER(client->response.headers, "connection", "close");

    const char *connection = l_get_header(client->response.headers, "connection");

    if (l_is_strcaseeq("close", connection))
        client->close_connection = TRUE;
    else if (l_is_strcaseeq("keep-alive", connection))
        client->close_connection = FALSE;

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
        parser_settings->on_headers_complete = _on_headers_complete;
        parser_settings->on_message_complete = _on_message_complete;
    }

    return parser_settings;
}

static int _do_http_parse(l_client_t *client, const char *at, size_t len)
{
    size_t nparsed = http_parser_execute(&client->parser, _get_http_parser_settings(), at, len);
    if (nparsed == len)
        return 0;

    LOG_ERROR_STR(HTTP_PARSER_ERRNO(&client->parser));
    return ERR_HTTP_PARSE;
}


static int _server_on_request(l_client_t *client)
{
    int err = 0;

    if (l_is_implemented_http_method(client)) {
        const char *url_path = l_get_url_path(client);
        l_route_match_t match = l_match_route(url_path, client->request.method);

        if (match.callback)
            client->response = match.callback(client, match.args);
        else
            client->response.status_code = 404;

        l_free_match(match);
        L_FREE(url_path);
    } else {
        client->response.status_code = 405;
    }

    return err;
}

static int _server_on_data(l_client_t *client, const char *buf, ssize_t nread)
{
    int err = 0;
    l_server_t *server = client->server;
    _(_do_http_parse(client, buf, nread));

    if (client->request.is_finished) {
        if (server->on_request_cb && client->response.status_code == 200)
            _(server->on_request_cb(client));

        _(l_send_response(client, &client->response));
        _log_request(client);
        if (client->response.callback)
            _(client->response.callback(&client->response));

        l_reset_connection(client);
    }

END:;
    if (err < 0) {
        int status_code = client->response.status_code;
        status_code = status_code ? status_code : 400;

        L_PUT_HEADER(client->response.headers, "content-type", "text/html");
        L_PUT_HEADER(client->response.headers, "connection", "close");

        l_send_response(client, &client->response);
        _log_request(client);
    }
    return err;
}

static void _server_on_read(uv_stream_t *client_handle, ssize_t nread, const uv_buf_t *buf)
{
    int err = 0;
    l_client_t *client = client_handle->data;
    l_server_t *server = client->server;

    // nread == 0 indicating that at this point there is nothing to be read.
    if (nread > 0) {
        if (server->on_data_cb)
            err = server->on_data_cb(client, buf->base, nread);
    } else if (nread < 0) {
        // connection closed by client
        if (nread == UV_EOF)
            client->close_connection = TRUE;
        else
            err = ERR_UV_READ;
    }

    L_FREE(buf->base);

    // if any error happend
    if (err < 0) {
        LOG_ERROR(err);
        client->close_connection = TRUE;
    }

    if (client->close_connection)
        l_close_connection(client);
}


static void _server_on_connect(uv_stream_t *server_handle, int status)
{
    l_client_t *client = l_create_connection((l_server_t *) server_handle->data);

    if (client == NULL) {
        LOG_ERROR(ERR_CREATE_CONNECTION);
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

    server->max_content_length = DEFAULT_MAX_CONTENT_LENGTH;

    return server;
}

void l_set_ip_port(l_server_t *server, const char *ip, int port)
{
    if (l_is_str(ip))
        server->ip = ip;
    if (port)
        server->port = port;
}

void l_start_server(l_server_t *server)
{
    struct sockaddr_in addr;

    UV_CHECK(uv_loop_init(&server->loop));
    UV_CHECK(uv_tcp_init(&server->loop, &server->handle));
    UV_CHECK(uv_ip4_addr(server->ip, server->port, &addr));
    UV_CHECK(uv_tcp_bind(&server->handle, (const struct sockaddr *) &addr, 0));
    UV_CHECK(uv_listen((uv_stream_t *) &server->handle, 128, _server_on_connect));

    l_log(" * Running on http://%s:%d/", server->ip, server->port);
    uv_run(&server->loop, UV_RUN_DEFAULT);

    uv_loop_close(&server->loop);
}
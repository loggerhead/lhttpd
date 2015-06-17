#include <lhttpd.h>

const char *on_http_request(l_client_t *client)
{
    l_log("==> %s", client->req.url);
    l_print_headers(client->req.headers);
    l_log("");
    if (client->req.method != HTTP_GET)
        l_log("%.*s\n", 80, client->req.body);

    l_send_body(client, "hello, world");
    // return empty string to tell caller everything is OK, return error desription othewise.
    return "";
}

int main(int argc, char *argv[])
{
    // Create a server instance listening at '0.0.0.0:9999'
    // You can use `l_set_ip_port` to change listening ip and port
    l_server_t *server = l_create_server();
    // Set http request callback function, `on_request_cb` will be called when request finished.
    server->on_request_cb = on_http_request;
    // Start `libuv` event loop
    l_start_server(server);
    return 0;
}
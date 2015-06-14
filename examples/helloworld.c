#include <lhttpd.h>

const char *on_request(l_client_t *client)
{
    l_log("==> %s", client->req.url);
    l_print_headers(client->req.headers);
    l_log("");
    if (client->req.method != L_HTTP_GET)
        l_log("%.*s\n", 80, client->req.body);

    l_send_body(client, "hello, world");
    return "";
}

int main(int argc, char *argv[])
{
    l_server_t *server = l_create_server();
    server->on_request_cb = on_request;
    l_start_server(server);
    return 0;
}
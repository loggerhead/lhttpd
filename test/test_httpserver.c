#include "../include/lhttpd.h"
#include <assert.h>

const char *on_request(client_t *client)
{
    l_log("==> %s", client->url);
    l_print_headers(client->headers);
    l_log("");
    if (client->parser.method != HTTP_GET)
        l_log("%.*s\n", 80, client->body);

    l_send_body(client, "hello, world");
    return "";
}

int main(int argc, char *argv[])
{
    server_t *server = l_get_server_instance();
    if (argc == 2)
        l_set_ip_port(server, NULL, atoi(argv[1]));
    else if (argc == 3)
        l_set_ip_port(server, argv[1], atoi(argv[2]));
    server->on_request_cb = on_request;

    l_start_server(server);
    return 0;
}
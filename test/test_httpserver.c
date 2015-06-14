#include "../include/lhttpd.h"
#include <assert.h>

const char *on_request(l_client_t *client)
{
    l_log("==> %s", client->http.url);
    l_print_headers(client->http.headers);
    l_log("");
    if (client->http.parser.method != HTTP_GET)
        l_log("%.*s\n", 80, client->http.body);

    l_send_body(client, "hello, world");
    return "";
}

int main(int argc, char *argv[])
{
    l_server_t *server = l_create_server();
    if (argc == 2)
        l_set_ip_port(server, NULL, atoi(argv[1]));
    else if (argc == 3)
        l_set_ip_port(server, argv[1], atoi(argv[2]));
    server->on_request_cb = on_request;

    l_start_server(server);
    return 0;
}
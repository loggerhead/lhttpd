#include "../include/lhttpd.h"
#include <assert.h>

const char *on_request(client_t *client)
{
    l_send_body(client, "hello, world");
    return "";
}

int main()
{
    server_t *server = l_get_server_instance();
    server->on_request_cb = on_request;

    l_start_server(server);
    return 0;
}
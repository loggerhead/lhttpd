#include <stdio.h>
#include <lhttpd.h>

const char *on_tcp_data(l_client_t *client, const char *data, ssize_t len)
{
    l_send_bytes(client, data, len);
    return "";
}

int main(int argc, char *argv[])
{
    l_server_t *server = l_create_server();
    server->on_data_cb = on_tcp_data;
    l_start_server(server);
    return 0;
}
#include <stdio.h>
#include <lhttpd.h>

int on_tcp_data(l_client_t *client, const char *data, ssize_t len)
{
    if (len > 500)
        len = 500;
    l_log("%.*s", len, data);
    l_send_bytes(client, data, len);
    return 0;
}

int main(int argc, char *argv[])
{
    l_server_t *server = l_create_server();
    server->on_data_cb = on_tcp_data;
    l_start_server(server);
    return 0;
}
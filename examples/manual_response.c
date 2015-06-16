#include <stdio.h>
#include <lhttpd.h>

const char *on_request(l_client_t *client)
{
    l_log("Please input your HTTP response:");
    char str[2] = {0};

    while (1) {
        int ch = getchar();
        if (ch == EOF)
            break;
        str[0] = ch;
        l_send_bytes(client, str, 1);
    }
    return "";
}

int main(int argc, char *argv[])
{
    l_server_t *server = l_create_server();
    server->on_request_cb = on_request;
    l_start_server(server);
    return 0;
}
#include "../include/lhttpd.h"
#include <assert.h>

#define BODY_LEN 100000

const char *on_request(l_client_t *client)
{
    char *body = l_malloc(BODY_LEN+1);
    memset(body, 'F', BODY_LEN);
    body[BODY_LEN] = '\0';

    l_send_body(client, body);

    L_FREE(body);
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
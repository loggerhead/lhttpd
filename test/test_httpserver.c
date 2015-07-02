#include "../lhttpd.h"
#include <assert.h>

#define BODY_LEN 100000

int isnum(const char *str)
{
    if (!(str && *str))
        return 0;

    while ('0' <= *str && *str <= '9')
        str++;

    return *str == '\0';
}

const char *on_request(l_client_t *client)
{
    if (l_is_http_post(client) && isnum(client->req.body)) {
        l_send_code(client, atoi(client->req.body));
    } else {
        // 'F' * BODY_LEN
        char *body = l_malloc(BODY_LEN+1);
        memset(body, 'F', BODY_LEN);
        body[BODY_LEN] = '\0';

        l_send_body(client, body);

        L_FREE(body);
    }

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
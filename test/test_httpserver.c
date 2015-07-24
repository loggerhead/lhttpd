#include "../lhttpd.h"
#include <assert.h>

#define TEST_DATA_LEN 104857

int is_num(const char *str)
{
    if (!(str && *str))
        return 0;

    while ('0' <= *str && *str <= '9')
        str++;

    return *str == '\0';
}

int on_request(l_client_t *client)
{
    if (l_is_http_post(client) && is_num(client->request.body)) {
        l_send_code(client, atoi(client->request.body));
    } else {
        // 'F' * TEST_DATA_LEN
        char *body = l_malloc(TEST_DATA_LEN+1);
        memset(body, 'F', TEST_DATA_LEN);
        body[TEST_DATA_LEN] = '\0';

        l_send_body(client, body);

        L_FREE(body);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    l_server_t *server = l_create_server();
    if (argc == 2)
        l_set_ip_port(server, NULL, atoi(argv[1]));
    else if (argc == 3)
        l_set_ip_port(server, argv[1], atoi(argv[2]));
    server->on_request_cb = on_request;

    server->max_content_length = TEST_DATA_LEN;

    l_start_server(server);
    return 0;
}
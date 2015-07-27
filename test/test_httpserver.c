#include "../lhttpd.h"
#include <assert.h>

#define TEST_DATA_LEN 1048576

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
    int err = 0;
    int status_code_class = client->response.status_code / 100;

    if (status_code_class != 4 && status_code_class != 5) {
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
    } else {
        err = l_send_response(client, &client->response);
        if (client->response.callback)
            err = client->response.callback(&client->response);
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
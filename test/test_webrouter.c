#include "../include/lhttpd.h"
#include <assert.h>

l_http_response_t home(l_client_t *client)
{
    l_http_response_t response = l_create_response();

    response.body = "hello world";

    return response;
}

l_http_response_t foo(l_client_t *client)
{
    l_http_response_t response = l_create_response();

    if (l_is_http_get(client)) {
        response.body = "foo";
    } else {
        response.status_code = 201;
    }

    return response;
}

int main(int argc, char *argv[])
{
    l_add_router("/", HTTP_GET, home);
    l_add_router("/foo", HTTP_GET, foo);
    l_add_router("/foo", HTTP_POST, foo);

    l_server_t *server = l_create_server();
    if (argc == 2)
        l_set_ip_port(server, NULL, atoi(argv[1]));
    else if (argc == 3)
        l_set_ip_port(server, argv[1], atoi(argv[2]));
    l_start_server(server);
    return 0;
}
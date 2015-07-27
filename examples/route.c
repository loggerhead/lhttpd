#include <lhttpd.h>

l_http_response_t home(l_client_t *client, l_hitem_t *args)
{
    const char *body = "hello world";
    l_http_response_t response = l_create_response();
    l_set_response_body(&response, body, strlen(body));
    return response;
}

l_http_response_t statics(l_client_t *client, l_hitem_t *args)
{
    const char *filepath = l_hget(args, "filepath");
    l_log("%s", filepath);
    return l_create_response();
}

l_http_response_t test(l_client_t *client, l_hitem_t *args)
{
    const char *test = l_hget(args, "test");
    l_log("%s", test);
    if (l_is_http_get(client)) {
        l_log("GET");
    } else {
        l_log("POST");
    }
    return l_create_response();
}

int main(int argc, char *argv[])
{
    l_add_route("/", HTTP_GET, home);
    l_add_route("/static/<filepath:path>", HTTP_GET, statics);
    l_add_route("/<test>", HTTP_GET, test);
    l_add_route("/<test>", HTTP_POST, test);

    l_server_t *server = l_create_server();
    l_start_server(server);
    return 0;
}

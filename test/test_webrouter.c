#include "../lhttpd.h"
#include <assert.h>

l_http_response_t test_static(l_client_t *client, l_hitem_t *args)
{
    l_http_response_t response = l_create_response();

    if (l_is_http_get(client)) {
        const char *body = "static";
        l_set_response_body(&response, body, strlen(body));
    } else {
        response.status_code = 204;
    }

    return response;
}

l_http_response_t test_dynamic(l_client_t *client, l_hitem_t *args)
{
    l_http_response_t response = l_create_response();

    const char *test = l_hget(args, "test");
    assert(!strcmp(test, "x"));

    return response;
}

l_http_response_t test_int(l_client_t *client, l_hitem_t *args)
{
    l_http_response_t response = l_create_response();

    const char *a = l_hget(args, "a");
    assert(atoi(a) == 1);

    return response;
}

l_http_response_t test_path(l_client_t *client, l_hitem_t *args)
{
    l_http_response_t response = l_create_response();

    const char *path = l_hget(args, "path");
    if (l_is_http_get(client)) {
        assert(!strcmp(path, "good/end"));
    } else {
        assert(!strcmp(path, "bad/"));
    }

    return response;
}

l_http_response_t test_all(l_client_t *client, l_hitem_t *args)
{
    l_http_response_t response = l_create_response();

    const char *in = l_hget(args, "in");
    int one = atoi(l_hget(args, "one"));
    const char *location = l_hget(args, "location");

    if (l_is_http_get(client)) {
        assert(!strcmp(in, "above"));
        assert(one == 1);
        assert(!location || !strcmp(location, "here"));

        const char *body = "hello world";
        l_set_response_body(&response, body, strlen(body));
    } else {
        assert(!strcmp(in, "below"));
        assert(one == 0);
        assert(!strcmp(location, "there"));
    }

    return response;
}

int main(int argc, char *argv[])
{
    // Static route
    l_add_route("/", HTTP_GET, test_static);
    l_add_route("/static", HTTP_GET, test_static);
    l_add_route("/static/", HTTP_GET, test_static);
    l_add_route("/static/test.html", HTTP_POST, test_static);
    // Dynamic route
    l_add_route("/<test>", HTTP_GET, test_dynamic);
    l_add_route("/<test>/", HTTP_GET, test_dynamic);
    l_add_route("/y/<test>", HTTP_GET, test_dynamic);
    l_add_route("/<test>/y", HTTP_POST, test_dynamic);
    // // Int filter
    l_add_route("/int/<a:int>", HTTP_GET, test_int);
    l_add_route("/<a:int>/int", HTTP_GET, test_int);
    // Path filter
    l_add_route("/path/to/<path:path>", HTTP_GET, test_path);
    l_add_route("/path/<path:path>", HTTP_POST, test_path);
    // All
    l_add_route("/<in>/<one:int>/test", HTTP_GET, test_all);
    l_add_route("/<in>/<one:int>/bar/<location:path>", HTTP_GET, test_all);
    l_add_route("/<in>/<one:int>/bar/<location:path>", HTTP_POST, test_all);

    l_server_t *server = l_create_server();
    if (argc == 2)
        l_set_ip_port(server, NULL, atoi(argv[1]));
    else if (argc == 3)
        l_set_ip_port(server, argv[1], atoi(argv[2]));

    l_start_server(server);

    return 0;
}

#include "../lhttpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int on_request(l_client_t *client)
{
    const char *url = client->request.url;
    const char *test_bytes = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    const char *test_body = "test";
    const int test_code = 204;
    l_http_response_t test_response = l_create_response();
    l_http_response_t test_redirect = l_create_redirect_response("/");
    l_http_response_t test_static_file = l_create_response_by_file("test_httputil.c");

    const char *path = l_get_url_path(client);
    assert(!strncmp(path, "/test", 5));
    L_FREE(path);

    if (!strcmp(url, "/test/send_bytes")) {
        l_send_bytes(client, test_bytes, strlen(test_bytes));
    } else if (!strcmp(url, "/test/send_response")) {
        l_send_response(client, &test_response);
    } else if (!strcmp(url, "/test/send_code")) {
        l_send_code(client, test_code);
    } else if (!strcmp(url, "/test/send_body")) {
        l_send_body(client, test_body);
    } else if (!strcmp(url, "/test/redirect")) {
        l_send_response(client, &test_redirect);
    } else if (!strcmp(url, "/test/static_file")) {
        l_send_response(client, &test_static_file);
    }

    l_client_t *c = l_create_connection(client->server);
    l_reset_connection(c);
    l_close_connection(c);

    return 0;
}

void start_server(int argc, char *argv[])
{
    l_server_t *server = l_create_server();
    if (argc == 2)
        l_set_ip_port(server, NULL, atoi(argv[1]));
    else if (argc == 3)
        l_set_ip_port(server, argv[1], atoi(argv[2]));
    server->on_request_cb = on_request;

    l_start_server(server);
}

int main(int argc, char *argv[])
{
    l_hitem_t *headers = NULL;
    l_hitem_t *_headers = NULL;
#define XX(key, value) _headers = l_hput(_headers, strdup(key), strdup(value))
    XX("Content-length", "123");
    XX("Host", "troydhanson.github.io");
    XX("Connection", "keep-alive");
    XX("Cache-Control", "max-age=0");
    XX("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    XX("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.152 Safari/537.36");
    XX("Referer", "https,//troydhanson.github.io/uthash/");
    XX("Accept-Encoding", "gzip, deflate, sdch");
    XX("Accept-Language", "zh-CN,zh;q=0.8");
    XX("If-Modified-Since", "Wed, 19 Nov 2014 03,01,22 GMT");
#undef XX

    L_HITER(_headers, header) {
        L_PUT_HEADER(headers, header->key, header->value);
    }

    assert(atoi(l_get_header(headers, "Content-length")) == 123);
    assert(!strcmp(l_get_header(headers, "Connection"), "keep-alive"));
    assert(!strcmp(l_get_header(headers, "If-Modified-Since"), "Wed, 19 Nov 2014 03,01,22 GMT"));
    l_free_headers(headers);

    assert(!strcmp(l_get_mimetype(".."), "application/octet-stream"));
    assert(!strcmp(l_get_mimetype(".jpg"), "application/octet-stream"));
    assert(!strcmp(l_get_mimetype("a.jpg"), "image/jpeg"));
    assert(!strcmp(l_get_mimetype("a.html"), "text/html"));
    assert(!strcmp(l_get_mimetype("a.htm"), "text/html"));

    const char *tmp = "hello";
    l_http_response_t response = l_create_response();
    l_set_response_body(&response, tmp, strlen(tmp));
    assert(!strcmp(response.body, tmp));
    L_FREE(response.body);

    start_server(argc, argv);

    return 0;
}
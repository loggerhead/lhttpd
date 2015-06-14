#include "httputil.h"
#include "util.h"


int l_has_error(const char *errmsg)
{
    return (errmsg && *errmsg);
}

l_hitem_t *l_add_header(l_hitem_t *headers, const char *field, const char *value)
{
    return l_hput(headers, field, value);
}

char *l_get_header(l_hitem_t *headers, const char *field)
{
    return l_hget(headers, field);
}

static void free_header(l_hitem_t *header)
{
    L_FREE(header->key);
    L_FREE(header->value);
}

void l_free_headers(l_hitem_t *headers)
{
    l_hfree(headers, free_header);
}

void l_print_headers(l_hitem_t *headers)
{
    l_hitem_t *header;

    for(header=headers; header; header=header->hh.next)
        l_log("%s: %s", header->key, header->value);
}

void l_convert_parser_method(l_client_t *client)
{
    switch(client->parser.method) {
        case HTTP_GET:
        case HTTP_POST:
        case HTTP_PUT:
        case HTTP_DELETE:
        case HTTP_HEAD:
            client->req.method = CONVERT_PARSER_METHOD(client->parser.method);
            break;
        default:
            client->req.method = L_HTTP_UNKNOWN;
            break;
    }
}

l_bool_t l_is_http_get(l_client_t *client)
{
    return client->req.method == L_HTTP_GET;
}

l_bool_t l_is_http_post(l_client_t *client)
{
    return client->req.method == L_HTTP_POST;
}

l_bool_t l_is_http_head(l_client_t *client)
{
    return client->req.method == L_HTTP_HEAD;
}

l_bool_t l_is_http_delete(l_client_t *client)
{
    return client->req.method == L_HTTP_DELETE;
}

l_bool_t l_is_http_put(l_client_t *client)
{
    return client->req.method == L_HTTP_PUT;
}
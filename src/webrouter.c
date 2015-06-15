#include "webrouter.h"

static l_bool_t is_static_route(const char *rule)
{
    return TRUE;
}

static const char *l_get_rule(const char *url)
{
    return url;
}

static l_hitem_t *_static_routers[MAX_HTTP_METHOD_NUM] = { NULL };

void l_add_router(const char *rule, l_http_method_t method, l_match_router_cb callback)
{
    if (is_static_route(rule)) {
        _static_routers[method] = l_hput(_static_routers[method], rule, (char *) callback);
    } else {

    }
}

l_match_router_cb l_match_router(const char *url, l_http_method_t method)
{
    const char *rule = l_get_rule(url);

    if (is_static_route(rule)) {
        return (l_match_router_cb) l_hget(_static_routers[method], rule);
    } else {
        return NULL;
    }
}
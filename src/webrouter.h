#ifndef _WEB_ROUTER_H
#define _WEB_ROUTER_H

#include "common.h"

typedef struct _dynamic_rule l_dynamic_rule_t;
typedef struct _route_node l_route_node_t;

typedef struct {
    const char *ptr;
    int len;
} l_token_t;

typedef enum {
    L_FILTER_NULL = 0,
    L_FILTER_INT,
    L_FILTER_PATH,
} l_filter_t;

// rule is node of route like `<action>`, `<name:filter>`, etc.
struct _dynamic_rule {
    l_token_t name;
    l_filter_t filter;
};

// route is url routing way, like `/<action>/<name:filter>/`
struct _route_node {
    // value
    l_match_route_cb callback;
    // children
    l_hitem_t *statics;
    l_hitem_t *dynamics;
};

void l_free_match(l_route_match_t match);
void l_free_routes();

#define NODE_GUARD ""

#endif /* _WEB_ROUTER_H */
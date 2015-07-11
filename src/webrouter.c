/* This file implements url route.
 *
 *          _roots
 * +-----+------+--------+
 * | GET | POST | ...... |
 * +--|--+------+--------+                            _dynamics
 *    |                                      +----------------+--------+
 *    v (children)             +------------>| name    filter | ...... |
 * +-----+---------------------|---+         +----------------+--------+
 * |token|callback statics dynamics|          l_dynamic_rule_t
 * +-----+-------------------------+
 *   key            value
 */

#include <stdio.h>
#include <assert.h>
#include "webrouter.h"

// String key : l_dynamic_route_t value
static l_hitem_t *_dynamics;
// String key : l_route_node_t value
static l_route_node_t _roots[MAX_HTTP_METHOD_NUM];


#define _PRINT_CHILDREN(nodes)                                  \
do {                                                            \
    L_HITER(nodes, item) {                                      \
        l_route_node_t *child = (l_route_node_t *) item->value; \
        children[tail++] = child;                               \
                                                                \
        if (child->callback)                                    \
            printf("[%s]\t", item->key);                        \
        else                                                    \
            printf("%s\t", item->key);                          \
    }                                                           \
} while (0)

// used for debug
void _print_route_nodes(l_route_node_t *node)
{
    int head = 0, tail = 0;
    l_route_node_t *children[1024] = { NULL };
    children[tail++] = node;

    l_log("--------- PRINT BEGIN ---------");
    while (head < tail) {
        node = children[head++];
        _PRINT_CHILDREN(node->statics);
        _PRINT_CHILDREN(node->dynamics);
        printf("\n");
    }
    l_log("---------  PRINT END  ---------");
}

#undef _PRINT_CHILDREN


static void _free_args(l_hitem_t *item)
{
    L_FREE(item->key);
    L_FREE(item->value);
}

void l_free_match(l_route_match_t match)
{
    l_hfree(match.args, _free_args);
}

static void _free_rule(l_hitem_t *item)
{
    L_FREE(item->value);
}

static void _free_routes(l_hitem_t *nodes)
{
    L_HITER(nodes, item) {
        l_route_node_t *node = (l_route_node_t *) item->value;
        _free_routes(node->statics);
        _free_routes(node->dynamics);

        L_FREE(item->value);
        L_FREE(item->key);
    }

    l_hfree(nodes, NULL);
}

void l_free_routes()
{
    l_hfree(_dynamics, _free_rule);

    for (int i = 0; i < MAX_HTTP_METHOD_NUM; i++) {
        _free_routes(_roots[i].statics);
        _free_routes(_roots[i].dynamics);
    }
}


/* Extract relative path from url
 * NOTE: need free
 */
static const char *_get_relative_url(const char *url)
{
    const char *head1 = "http://";
    const char *head2 = "https://";
    int hlen1 = strlen(head1);
    int hlen2 = strlen(head2);

    // skip host part
    if (!strncmp(url, head1, hlen1))
        url = strchr(url + hlen1, '/');
    else if (!strncmp(url, head2, hlen2))
        url = strchr(url + hlen2, '/');
    if (!url)
        url = "/";

    const char *tail = strchr(url, '?');
    return tail ? strndup(url, tail-url) : strdup(url);
}

/* split `<name:filter>` to `name` and `filter` parts
 * NOTE: need free
 */
static l_dynamic_rule_t *_create_dynamic_rule(const char *token)
{
    l_dynamic_rule_t *rule = l_calloc(1, sizeof(*rule));
    // discard first '<' and last '>'
    int len = strlen(++token)-1;

    const char *filter = strchr(token, ':');

    rule->name.ptr = token;
    // <name>
    if (!filter) {
        rule->name.len = len;
    // <name:filter>
    } else {
        rule->name.len = (filter++) - token;

        if (!strncasecmp(filter, "int", 3))
            rule->filter = L_FILTER_INT;
        else if (!strncasecmp(filter, "path", 4))
            rule->filter = L_FILTER_PATH;
        else
            rule->filter = L_FILTER_NULL;
    }

    return rule;
}

/* split url by '/'. For example, '/hello/world' ==> '/' 'hello' '/' 'world'
 * `_get_token(token.ptr + token.len)` will parse next token which include '/'
 */
static l_token_t _get_token(const char *url)
{
    l_token_t token = { url, 0 };

    if (!l_has_str(url))
        return token;

    if (url[0] == '/') {
        token.len = 1;
        return token;
    }

    /* '/foo/more'
     *   ^  <----
     * 'more'
     *  ^
     */
    const char *sep = strchr(url, '/');
    token.len = sep ? sep-url : strlen(url);

    return token;
}

static l_bool_t _is_dynamic_rule(const char *str, int len)
{
    return str[0] == '<' && str[len-1] == '>';
}


// NOTE: need free after server close ==> stoken, nodes (not root in _roots), rule in _dynamics
void l_add_route(const char *route, l_http_method_t method, l_match_route_cb callback)
{
    l_token_t token = _get_token(route);
    if (!token.ptr) {
        l_error("ERROR: Invalid url route!");
        exit(1);
    }

    l_route_node_t *root = &_roots[method];
    l_route_node_t *leaf = NULL;
    const char *stoken = NULL;

    do {
        stoken = strndup(token.ptr, token.len);

        leaf = (l_route_node_t *) l_hget(root->statics, stoken);
        if (!leaf)
            leaf = (l_route_node_t *) l_hget(root->dynamics, stoken);

        // Add new node if not exists in tree
        if (!leaf) {
            leaf = l_calloc(1, sizeof(*leaf));
            // Dynamic route
            if (_is_dynamic_rule(token.ptr, token.len)) {
                l_dynamic_rule_t *rule = _create_dynamic_rule(stoken);
                L_HPUT(_dynamics, stoken, rule);
                L_HPUT(root->dynamics, stoken, leaf);
            // Static route
            } else {
                L_HPUT(root->statics, stoken, leaf);
            }
        } else {
            L_FREE(stoken);
        }

        root = leaf;
        token = _get_token(token.ptr + token.len);
    } while (token.ptr && token.len);

    leaf->callback = callback;
}

// NOTE: need free after callback ==> `name` and `stoken` in `args[name] = stoken`
static l_bool_t _match_route(l_route_match_t *match, const char *url, l_route_node_t *root)
{
    l_token_t token = _get_token(url);

    if (!l_has_str(token.ptr)) {
        match->callback = root->callback;
        return TRUE;
    }

    const char *stoken = strndup(token.ptr, token.len);
    const char *remain = token.ptr + token.len;

    l_match_route_cb saved = match->callback;

    // match statics
    l_route_node_t *child = (l_route_node_t *) l_hget(root->statics, stoken);
    if (child && _match_route(match, remain, child)) {
        L_FREE(stoken);
        goto GOOD_END;
    }

    // match dynamic
    L_HITER(root->dynamics, item) {
        match->callback = saved;

        child = (l_route_node_t *) item->value;
        l_dynamic_rule_t *rule = (l_dynamic_rule_t *) l_hget(_dynamics, item->key);
        const char *name = strndup(rule->name.ptr, rule->name.len);

#define _RETURN_AND_SAVE_ARG(arg)                              \
        do {                                                   \
            L_HPUT(match->args, name, arg);                    \
            goto GOOD_END;                                     \
        } while (0)

        switch (rule->filter) {
            case L_FILTER_INT:
                if (!l_is_num(stoken))
                    break;
                // else Fall down
            case L_FILTER_NULL:
                if (_match_route(match, remain, child))
                    _RETURN_AND_SAVE_ARG(stoken);
                break;
            case L_FILTER_PATH:
                match->callback = child->callback;
                _RETURN_AND_SAVE_ARG(strdup(url));
        }
#undef _RETURN_AND_SAVE_ARG

        L_FREE(name);
    }

    // Bad end
    match->callback = saved;
    L_FREE(stoken);
    return FALSE;
GOOD_END:
    return TRUE;
}

l_route_match_t l_match_route(const char *url, l_http_method_t method)
{
    url = _get_relative_url(url);
    l_route_node_t *root = &_roots[method];
    l_route_match_t match = { NULL, NULL };

    if (url)
        _match_route(&match, url, root);

    L_FREE(url);
    return match;
}
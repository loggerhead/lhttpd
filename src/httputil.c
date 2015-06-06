#include "httputil.h"
#include "util.h"


int l_has_error(const char *errmsg)
{
    return (errmsg && *errmsg);
}

hitem_t *l_add_header(hitem_t *headers, const char *field, const char *value)
{
    return l_hput(headers, field, value);
}

char *l_get_header(hitem_t *headers, const char *field)
{
    return l_hget(headers, field);
}

static void free_header(hitem_t *header)
{
    L_FREE(header->key);
    L_FREE(header->value);
}

void l_free_headers(hitem_t *headers)
{
    l_hfree(headers, free_header);
}

void l_print_headers(hitem_t *headers)
{
    hitem_t *header;

    for(header=headers; header; header=header->hh.next)
        l_log("%s: %s", header->key, header->value);
}
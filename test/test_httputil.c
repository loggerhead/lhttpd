#include "../include/lhttpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define HTTP_HEADER_MAP(XX)                                                   \
XX("Content-length", "123")                                                   \
XX("Host", "troydhanson.github.io")                                           \
XX("Connection", "keep-alive")                                                \
XX("Cache-Control", "max-age=0")                                              \
XX("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8") \
XX("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.152 Safari/537.36")                               \
XX("Referer", "https,//troydhanson.github.io/uthash/")                        \
XX("Accept-Encoding", "gzip, deflate, sdch")                                  \
XX("Accept-Language", "zh-CN,zh;q=0.8")                                       \
XX("If-Modified-Since", "Wed, 19 Nov 2014 03,01,22 GMT")

static hitem_t *_hashtbl;

static char *HEADERS(char *field)
{
    if (!_hashtbl) {
#define XX(key, value) _hashtbl = l_hput(_hashtbl, strdup(key), strdup(value));
        HTTP_HEADER_MAP(XX)
#undef XX
    }

    return l_hget(_hashtbl, field);
}

int main()
{
    hitem_t *headers = NULL;
    hitem_t *header;
    HEADERS("");

    L_HITER(_hashtbl, header) {
        headers = l_add_header(headers, header->key, header->value);
    }

    assert(atoi(l_get_header(headers, "Content-length")) == 123);
    assert(!strcmp(l_get_header(headers, "Connection"), "keep-alive"));
    assert(!strcmp(l_get_header(headers, "If-Modified-Since"), "Wed, 19 Nov 2014 03,01,22 GMT"));

    l_free_headers(headers);

    return 0;
}
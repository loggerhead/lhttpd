#include "../include/lhttpd.h"
#include <stdio.h>
#include <assert.h>

int main()
{
    char buf[8192];
    const char *str = "hello, world";
    char *tmp;

    l_error(str);
    L_FREE(l_malloc(sizeof(str)));
    L_FREE(l_calloc(1, sizeof(str)));

    sprintf(buf, "sizeof(\"%s\")=%lu", str, sizeof(str));
    tmp = l_mprintf("sizeof(\"%s\")=%lu", str, sizeof(str));
    assert(!strcmp(tmp, buf));
    l_warn(tmp);
    L_FREE(tmp);

    assert(l_strnchr(str, 'l', strlen(str)) == str+2);

    tmp = l_strdup(str);
    assert(!strcmp(tmp, str));

    l_repchr(tmp, 'l', 'R');
    assert(!strcmp(tmp, "heRRo, worRd"));
    l_log(tmp);

    l_lowercase(tmp);
    assert(!strcmp(tmp, "herro, worrd"));
    l_log(tmp);
    L_FREE(tmp);

    l_hitem_t *hashtbl = NULL;
    hashtbl = l_hput(hashtbl, "key", "value");
    char *value = l_hget(hashtbl, "key");
    assert(!strcmp(value, "value"));
    l_hitem_t *item = NULL;
    L_HITER(hashtbl, item) {
        assert(!strcmp(item->value, "value"));
    }
    l_hfree(hashtbl, NULL);

    hashtbl = NULL;
    L_HITER(hashtbl, item) {
    }

    return 0;
}
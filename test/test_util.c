#include "../lhttpd.h"
#include <stdio.h>
#include <assert.h>

int main()
{
    char buf[8192];
    const char *str = "hello, world";
    char *tmp;

    L_FREE(l_malloc(sizeof(str)));
    L_FREE(l_calloc(1, sizeof(str)));

    // string
    assert(l_is_num("12"));
    assert(!l_has_str(""));

    sprintf(buf, "sizeof(\"%s\")=%lu", str, sizeof(str));
    tmp = l_mprintf("sizeof(\"%s\")=%lu", str, sizeof(str));
    assert(!strcmp(tmp, buf));
    L_FREE(tmp);

    assert(l_strnchr(str, 'l', strlen(str)) == str+2);

    tmp = l_strdup(str);
    assert(!strcmp(tmp, str));

    l_repchr(tmp, 'l', 'R');
    assert(!strcmp(tmp, "heRRo, worRd"));

    l_lowercase(tmp);
    assert(!strcmp(tmp, "herro, worrd"));
    L_FREE(tmp);

    // file
    assert(l_match_file_suffix("a.out", "out"));
    assert(l_is_file_exist("test_util.c"));

    FILE *fp = fopen("test_util.c", "r");
    assert(l_get_filesize(fp));
    fclose(fp);

    const char *path = l_pathcat("hello", "world");
    assert(!strcmp(path, "hello/world"));
    L_FREE(path);

    const char *filename = l_url2filename("http://localhost:9999/a.js?q=test");
    assert(!strcmp(filename, "a.js"));
    L_FREE(filename);

    tmp = "hello/world.us";
    assert(!strcmp(l_get_dirname(tmp), "hello"));
    assert(!strcmp(l_get_basename(tmp), "world.us"));
    assert(!strcmp(l_get_suffix(tmp), "us"));

    // hash table
    l_hitem_t *hashtbl = NULL;
    L_HPUT(hashtbl, "key", "value");
    char *value = l_hget(hashtbl, "key");
    assert(!strcmp(value, "value"));
    L_HITER(hashtbl, item) {
        assert(!strcmp(item->value, "value"));
    }
    l_hfree(hashtbl, NULL);

    hashtbl = NULL;
    L_HITER(hashtbl, item) {
    }

    return 0;
}
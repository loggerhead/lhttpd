#include "../lhttpd.h"
#include <assert.h>

int main(int argc, char *argv[])
{
    const char *text = "hello, world";

    l_redis_connection_t conn = l_create_redis_connection(NULL, 0);

    l_redis_set(conn, "foo", text);

    const char *val = l_redis_get(conn, "foo");
    assert(!strcmp(val, text));
    L_FREE(val);

    l_close_redis_connection(conn);

    return 0;
}
#include "../lhttpd.h"
#include <assert.h>

int main(int argc, char *argv[])
{
#if HAS_SQLITE3
    const char *text = "hello, world";

    l_db_t db = l_create_db("test.db");

    l_exec_db(db, "insert into foo (key) values ('%q')", text);

    l_query_t *query = l_query_db(db, "select * from foo where key = '%q'", text);
    char *key, *val;
    L_DB_FOREACH_ROW(query, key, val) {
        assert(key && !strcmp(key, "key"));
        assert(val && !strcmp(val, text));
    }
    l_free_query(query);

    assert(l_is_exist_db(db, "select * from foo where key = '%q'", text));
    l_exec_db(db, "delete from foo");
    assert(!l_is_exist_db(db, "select * from foo where key = '%q'", text));

    l_close_db(db);
#endif
    return 0;
}

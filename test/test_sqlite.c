#include "../lhttpd.h"
#include <assert.h>

int main(int argc, char *argv[])
{
#if HAS_SQLITE3
    const char *text = "hello, world";

    l_db_t db = l_create_db("test.db");

    l_exec_db(db, "drop table if exists foo");
    l_exec_db(db, "create table foo ( key )");
    l_exec_db(db, "insert into foo (key) values ('%q')", text);

    l_query_t *query = l_query_db(db, "select * from foo where key = '%q'", text);
    char *key, *val;
    L_DB_FOREACH_ROW(query, i)
        L_DB_FOREACH_COL(query, i, key, val) {
            assert(key && !strcmp(key, "key"));
            assert(val && !strcmp(val, text));
        }

#if HAS_JSON_C
    l_json_t *jobj = l_query_to_json_object(query);
    assert(!strcmp(l_json_to_string(jobj), "{ \"key\": \"hello, world\" }"));
    l_free_json(jobj);

    l_json_t *array = l_query_to_json_array(query);
    assert(!strcmp(l_json_to_string(array), "[ { \"key\": \"hello, world\" } ]"));
    l_free_json(array);
#endif

    assert(l_is_exist_db(db, "select * from foo where key = '%q'", text));
    l_exec_db(db, "delete from foo");
    assert(!l_is_exist_db(db, "select * from foo where key = '%q'", text));

    l_free_query(query);
    l_close_db(db);
#endif
    return 0;
}

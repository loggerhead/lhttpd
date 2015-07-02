#include "sqlite.h"

l_db_t l_create_db(const char *dbpath)
{
    l_db_t db = NULL;

    if (SQLITE_OK != sqlite3_open(dbpath, &db))
        l_error("open %s failed: %s", dbpath, sqlite3_errmsg(db));

    return db;
}

void l_close_db(l_db_t db)
{
    if (NULL != db)
        sqlite3_close(db);
}

l_bool_t l_exec_db(l_db_t db, const char *sqlfmt, ...)
{
    char *sql;
    SQL_FORMAT(sql, sqlfmt);

    char *errmsg = NULL;
    l_bool_t res = (sqlite3_exec(db, sql, NULL, NULL, &errmsg) == SQLITE_OK);
    if (!res)
        l_warn("%s: %s", __func__, errmsg);

    sqlite3_free(errmsg);
    sqlite3_free(sql);
    return res;
}

void l_free_query(l_query_t *query)
{
    sqlite3_free(query);
}

static l_query_t *_query_db(l_db_t db, char *sql)
{
    char *errmsg = NULL;
    l_query_t *res = (l_query_t *)sqlite3_malloc(sizeof(*res));

    res->is_success = (sqlite3_get_table(db, sql, &res->results, &res->row, &res->col, &errmsg) == SQLITE_OK);
    if (!res->is_success)
        l_warn("%s: %s", __func__, errmsg);

    sqlite3_free(errmsg);
    sqlite3_free(sql);
    return res;
}

l_query_t *l_query_db(l_db_t db, const char *sqlfmt, ...)
{
    char *sql;
    SQL_FORMAT(sql, sqlfmt);

    return _query_db(db, sql);
}

l_bool_t l_is_exist_db(l_db_t db, const char *sqlfmt, ...)
{
    char *sql;
    SQL_FORMAT(sql, sqlfmt);

    l_query_t *query = _query_db(db, sql);
    l_bool_t res = HAS_QUERY_RESULT(query);
    l_free_query(query);
    return res;
}

// TODO: finish this
/*
json_object *l_get_json_db(l_db_t db, const char *sqlfmt, ...)
{
    char *sql;
    SQL_FORMAT(sql, sqlfmt);

    l_query_t *query = _query_db(db, sql);
    json_object *jobj = NULL;

    if (HAS_QUERY_RESULT(query)) {
        tCString key;
        tCString val;
        jobj = json_object_new_object();

        DB_FOREACH_COL(query, 0, key, val) {
            json_object *s = json_object_new_string(val ? val : "");
            json_object_object_add(jobj, key, s);
        }
    }

    sqlite3_free(query);
    return jobj;
}

json_object *db_get_json_array(l_db_t db, const char *sqlfmt, ...)
{
    char *sql;
    va_list args;
    va_start(args, sqlfmt);
    sql = sqlite3_vmprintf(sqlfmt, args);
    va_end(args);

    DBDATA *result = _db_get(sql);
    json_object *jarr = NULL;

    if (result && result->results && result->row && result->col) {
        tCString key;
        tCString val;
        jarr = json_object_new_array();

        for (int r = 0; r < result->row; r++) {
            json_object *jobj = json_object_new_object();
            DB_FOREACH_COL(result, r, key, val) {
                json_object_object_add(jobj, key, json_object_new_string(val ? val : ""));
            }
            json_object_array_add(jarr, jobj);
        }
    }

    sqlite3_free(result);
    sqlite3_free(sql);

    return jarr;
}
*/
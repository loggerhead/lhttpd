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

l_json_t *l_query_to_json_object(l_query_t *query)
{
#if HAS_JSON_C
    if (!HAS_QUERY_RESULT(query))
        return NULL;

    json_object *jobj = l_create_json_object();
    const char *key;
    const char *val;

    L_DB_FOREACH_COL(query, 0, key, val) {
        json_object *tmp = json_object_new_string(val ? val : "");
        json_object_object_add(jobj, key, tmp);
    }

    return jobj;
#else
    l_error("%s: not support json operation", __func__);
    return NULL;
#endif
}

l_json_t *l_query_to_json_array(l_query_t *query)
{
#if HAS_JSON_C
    if (!HAS_QUERY_RESULT(query))
        return NULL;

    json_object *array = l_create_json_array();

    L_DB_FOREACH_ROW(query, i) {
        json_object *jobj = json_object_new_object();

        const char *key;
        const char *val;
        L_DB_FOREACH_COL(query, i, key, val) {
            l_json_add_string(jobj, key, val ? val : "");
        }

        l_array_add_jobj(array, jobj);
    }

    return array;
#else
    l_error("%s: not support json operation", __func__);
    return NULL;
#endif
}
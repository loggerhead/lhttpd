#ifndef L_SQLITE_H

#include "common.h"

#define SQL_FORMAT(sql, sqlfmt)                         \
    do {                                                \
        va_list args;                                   \
        va_start(args, sqlfmt);                         \
        sql = sqlite3_vmprintf(sqlfmt, args);           \
        va_end(args);                                   \
    } while (0)

#define HAS_QUERY_RESULT(query) (query && query->results && (query->row > 0) && (query->col > 0))

#define L_SQLITE_H
#endif
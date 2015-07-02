#include "redis.h"

l_redis_connection_t l_create_redis_connection(const char *host, int port)
{
    if (!host)
        host = DEFAULT_REDIS_HOST;
    if (!port)
        port = DEFAULT_REDIS_PORT;

    l_redis_connection_t conn;
    conn.conn = redisConnect(host, port);
    conn.type = L_SYNC;

    if (IS_CONNECT_ERROR(conn.conn)) {
        if (conn.conn) {
            l_error("%s: %s", __func__, conn.conn->errstr);
        } else {
            l_error("%s: can't allocate redis context", __func__);
        }
    }

    return conn;
}

void l_close_redis_connection(l_redis_connection_t conn)
{
    if (conn.conn) 
        redisFree(conn.conn);
}


void l_redis_set(l_redis_connection_t conn, const char *key, const char *value)
{
    redisReply *r = redisCommand(conn.conn,"SET %s %s", key, value);
    freeReplyObject(r);
}

const char *l_redis_get(l_redis_connection_t conn, const char *key)
{
    redisReply *r = redisCommand(conn.conn,"GET %s", key);
    const char *value = NULL;
    if (r)
        value = strdup(r->str);
    freeReplyObject(r);
    return value;
}
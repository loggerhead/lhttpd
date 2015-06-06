#ifndef L_HTTP_CLIENT_H

#include "common.h"

char *l_get_ip(client_t *client, char ip[]);

void l_free_client(client_t *client);
void l_reset_client(client_t *client);
client_t *l_get_client_instance(server_t *server);

const char *l_send_bytes(client_t *client, const char *bytes, size_t len);
const char *l_send_response(client_t *client, const char *status_code, hitem_t *headers, const char *body);
const char *l_send_code(client_t *client, const char *status_code);
const char *l_send_body(client_t *client, const char *body);

const char *l_status_code(const char *code);
char *l_generate_response(client_t *client, const char *status_code, hitem_t *headers, const char *body);

#define HTTP_STATUS_CODE_MAP(XX)             \
XX("100", "Continue")                        \
XX("101", "Switching Protocols")             \
XX("200", "OK")                              \
XX("201", "Created")                         \
XX("202", "Accepted")                        \
XX("203", "Non-Authoritative Information")   \
XX("204", "No Content")                      \
XX("205", "Reset Content")                   \
XX("206", "Partial Content")                 \
XX("300", "Multiple Choices")                \
XX("301", "Moved Permanently")               \
XX("302", "Found")                           \
XX("303", "See Other")                       \
XX("304", "Not Modified")                    \
XX("305", "Use Proxy")                       \
XX("307", "Temporary Redirect")              \
XX("400", "Bad Request")                     \
XX("401", "Unauthorized")                    \
XX("402", "Payment Required")                \
XX("403", "Forbidden")                       \
XX("404", "Not Found")                       \
XX("405", "Method Not Allowed")              \
XX("406", "Not Acceptable")                  \
XX("407", "Proxy Authentication Required")   \
XX("408", "Request Time-out")                \
XX("409", "Conflict")                        \
XX("410", "Gone")                            \
XX("411", "Length Required")                 \
XX("412", "Precondition Failed")             \
XX("413", "Request Entity Too Large")        \
XX("414", "Request-URI Too Large")           \
XX("415", "Unsupported Media Type")          \
XX("416", "Requested range not satisfiable") \
XX("417", "Expectation Failed")              \
XX("500", "Internal Server Error")           \
XX("501", "Not Implemented")                 \
XX("502", "Bad Gateway")                     \
XX("503", "Service Unavailable")             \
XX("504", "Gateway Time-out")                \
XX("505", "HTTP Version not supported")

#define L_HTTP_CLIENT_H
#endif

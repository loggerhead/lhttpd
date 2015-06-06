#ifndef L_HTTP_SERVER_H

#include "common.h"

server_t *l_get_server_instance();
void l_set_ip_port(server_t *server, const char *ip, int port);
void l_start_server(server_t *server);

#define L_HTTP_SERVER_H
#endif

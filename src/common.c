#include <assert.h>
#include "common.h"

const char *_strerror(int err)
{
    static const char *_errmsgs[MAX_ERROR_CODE] = { 0 };

    if (_errmsgs[0] == NULL) {
#define XX(macro, code, message) _errmsgs[code] = message;
        ERR_MSG_MAP(XX)
#undef XX
    }

    err = -err;
    assert(0 <= err && err < MAX_ERROR_CODE);

    return _errmsgs[err];
}

void _log_request(l_client_t *client)
{
    char client_address[17] = { 0 };
    struct sockaddr sockname;
    int namelen = sizeof(sockname);
    // get string of client address
    uv_tcp_getpeername((uv_tcp_t*) &client->handle, &sockname, &namelen);
    uv_ip4_name((const struct sockaddr_in *) &sockname, client_address, sizeof(client_address));

    l_log("%s - - [%s] %d - \"%s %s\" ",
          client_address, l_now(),
          client->response.status_code,
          http_method_str(client->request.method), client->request.url);
}
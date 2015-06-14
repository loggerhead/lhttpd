#Lhttpd
[![Travis Build Status](https://travis-ci.org/loggerhead/lhttpd.svg)](https://travis-ci.org/loggerhead/lhttpd)

#Dependency
* [libuv](https://github.com/libuv/libuv)
* [cmake](http://www.cmake.org/)

##Test Dependency
if you are **linux** user, run `sudo apt-get install python-dev` first.

```shell
sudo pip install requests grequests
```

#Install
Below commands will compile and move files to `/usr/local/lib` and `/usr/local/include`:

```shell
./install.sh
```

**NOTE**: If you are linux user, please run beblow commands to update your shared libraries.

```shell
sudo echo "/usr/local/lib" >> /etc/ld.so.conf
sudo ldconfig
```

#Usage
See [examples](https://github.com/loggerhead/lhttpd/tree/master/examples).

```c
#include <lhttpd.h>

const char *on_request(l_client_t *client)
{
    l_log("==> %s", client->url);
    l_print_headers(client->headers);
    l_log("");
    if (client->parser.method != HTTP_GET)
        l_log("%.*s\n", 80, client->body);

    l_send_body(client, "hello, world");
    return "";
}

int main(int argc, char *argv[])
{
    l_server_t *server = l_create_server();
    server->on_request_cb = on_request;
    l_start_server(server);
    return 0;
}
```

compile and run!

```shell
gcc -o foo examples/helloworld.c -llhttpd && ./foo
```

#TODO
* [ ] 增加url route（bottle）
* [ ] 增加json/redis/sqlite支持
* [ ] 完成API文档
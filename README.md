#Lhttpd
[![Travis Build Status](https://travis-ci.org/loggerhead/lhttpd.svg)](https://travis-ci.org/loggerhead/lhttpd)

#Dependency
* [libuv](https://github.com/libuv/libuv)
* [cmake](http://www.cmake.org/)

#Install
Below commands will compile and move files to `/usr/local/lib` and `/usr/local/include`:

```shell
./install make
sudo make install
```

#Test
##Dependency
if you are **linux** user, run `sudo apt-get install python-dev` first.

```shell
sudo pip install grequests
```

##run
```shell
./install.sh test
```

#Usage
Create a `foo.c` and type into below codes:

```c
#include <lhttpd.h>

const char *on_request(client_t *client)
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
    server_t *server = l_get_server_instance();
    server->on_request_cb = on_request;
    l_start_server(server);
    return 0;
}
```

compile and run!

```shell
gcc -o foo foo.c -L/usr/local/lib -I/usr/local/include -llhttpd -luv && ./foo
```
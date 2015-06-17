#Lhttpd
[![Travis Build Status](https://travis-ci.org/loggerhead/lhttpd.svg)](https://travis-ci.org/loggerhead/lhttpd)

**NOTE**: Lhttpd is not thread-safe.

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
* [ ] 简化api (如: remove `l_` if static)，完成API文档
* [ ] 支持 `json`
* [ ] 支持 `redis`/`sqlite`
* [ ] 处理 `multipart/form-data`/`application/x-www-form-urlencoded`/`application/json`，参考 [四种常见的 POST 提交数据方式](https://www.imququ.com/post/four-ways-to-post-data-in-http.html#toc-2)、[Form-based File Upload in HTML](https://www.ietf.org/rfc/rfc1867.txt)
* [ ] 支持 `Transfer-Encoding`
* [ ] 支持 `gzip` 等压缩
#Lhttpd
[![Travis Build Status](https://travis-ci.org/loggerhead/lhttpd.svg)](https://travis-ci.org/loggerhead/lhttpd)

**NOTE**: Lhttpd is not thread-safe.

#Build and Install
##Dependency
You need [cmake](http://www.cmake.org/) and [make](http://www.gnu.org/software/make/) for build, and below libraries for compile.

* Required: [libuv](https://github.com/libuv/libuv);
* Optional: [sqlite3](https://www.sqlite.org/) for sqlite support;
* Optional: [redis](https://github.com/antirez/redis) and [hiredis](https://github.com/redis/hiredis) for redis support;
* Optional: [grequests](https://github.com/kennethreitz/grequests) for test;
* Optional: [valgrind](http://valgrind.org/) for memory leak check and [siege](https://www.joedog.org/siege-home/) for pressure test;

###libuv
```shell
git clone https://github.com/libuv/libuv.git
cd libuv
sh autogen.sh && sh autogen.sh
./configure
make
make check
sudo make install
```

###sqlite3
```shell
sudo apt-get install sqlite3 libsqlite3-dev
```

###hiredis
```shell
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo make install
```

If something error, maybe you need modify `adapters/libuv.h` as follow:

1. add `#include <stdlib.h>` to first line;
2. find `static int redisLibuvAttach(redisAsyncContext* ac, uv_loop_t* loop)` and delete `static`;

Then, re-compile.

###grequests
```shell
# Linux user run `sudo apt-get install python-dev` first
sudo pip install grequests
```

##Install
Below commands will compile and move files to `/usr/local/lib` and `/usr/local/include`:

```shell
./install.sh
```

**NOTE**: If you are linux user, please run beblow commands updating your shared libraries for using `lhttpd`.

```shell
sudo echo "/usr/local/lib" >> /etc/ld.so.conf
sudo ldconfig
```

#Usage
Run `./install.sh examples` to compile all examples, see [examples](https://github.com/loggerhead/lhttpd/tree/master/examples) for details. 

##API
You can use everything in [lhttpd.in.h](https://github.com/loggerhead/lhttpd/blob/master/include/lhttpd.in.h).

##Request Routing
Inspired by the [bottle](http://bottlepy.org/docs/dev/tutorial.html#request-routing).

#TODO
* [ ] 支持 `json`
* [ ] 处理 `multipart/form-data`/`application/x-www-form-urlencoded`/`application/json`，参考 [四种常见的 POST 提交数据方式](https://www.imququ.com/post/four-ways-to-post-data-in-http.html#toc-2)、[Form-based File Upload in HTML](https://www.ietf.org/rfc/rfc1867.txt)
* [ ] 支持 `Transfer-Encoding`
* [ ] 支持 `gzip` 等压缩
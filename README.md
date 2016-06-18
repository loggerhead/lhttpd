#Abandoned!
Because I find a good alternate--[Mongoose](https://github.com/cesanta/mongoose), which is small, no dependency, cross-platform, and written by pure C. Maybe you should consider use it.

#Lhttpd
[![Travis Build Status](https://travis-ci.org/loggerhead/lhttpd.svg)](https://travis-ci.org/loggerhead/lhttpd)

Lhttpd is a C library to take care of the detail about TCP and HTTP, and it's API is designed for ease of use, so you can focus on business logic.

#Features
* Asynchronous.
* Lightweight and easy to use.
* Provide TCP support and some webserver functionality.
* Simple [bottle-like](http://bottlepy.org/docs/dev/tutorial.html#request-routing) web route support.
* JSON support.
* Redis support.
* SQLite3 support.
* __NOT thread-safe__.

#Build and Install
##Install
```shell
# compile, generate `liblhttpd.xxx` and `lhttpd.h`, and move they to `/usr/local/lib` and `/usr/local/include`
./install.sh
```

**NOTE**: If you are linux user, please run below commands to update shared libraries before using `lhttpd`.

```shell
# You should use root user
echo "/usr/local/lib" >> /etc/ld.so.conf
ldconfig
```

##Dependency
You need [cmake](http://www.cmake.org/) and [make](http://www.gnu.org/software/make/) for build, and below libraries for compile.

* Required: [libuv](https://github.com/libuv/libuv).
* Optional: [json-c](https://github.com/json-c/json-c) for json support.
* Optional: [sqlite3](https://www.sqlite.org/) for sqlite support.
* Optional: [redis](https://github.com/antirez/redis) and [hiredis](https://github.com/redis/hiredis) for redis support.
* Optional: [grequests](https://github.com/kennethreitz/grequests) for test.
* Optional: [valgrind](http://valgrind.org/) for memory leak check and [siege](https://www.joedog.org/siege-home/) for pressure test.

###Required
```shell
# cmake and make
sudo apt-get install cmake make
# libuv
git clone https://github.com/libuv/libuv.git
cd libuv
sh autogen.sh && sh autogen.sh
./configure
make
make check
sudo make install
```

###Optional
```shell
# json-c
sudo apt-get install libjson0 libjson0-dev
# sqlite3
sudo apt-get install sqlite3 libsqlite3-dev
# redis
sudo apt-get install redis-server
# hiredis
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo make install
# grequests
sudo pip install grequests
```

####If install hiredis error
modify `adapters/libuv.h` as follow and try again:

1. add `#include <stdlib.h>` to first line.
2. find `static int redisLibuvAttach(redisAsyncContext* ac, uv_loop_t* loop)` and delete `static`.

#Usage
see [examples](https://github.com/loggerhead/lhttpd/tree/master/examples) or [test](https://github.com/loggerhead/lhttpd/tree/master/test) for details.

```shell
./install.sh examples
./install.sh test
```

##API
see defines in [lhttpd.in.h](https://github.com/loggerhead/lhttpd/blob/master/include/lhttpd.in.h).

##Request Routing
Inspired by the [bottle](http://bottlepy.org/docs/dev/tutorial.html#request-routing).

#LICENSE
MIT

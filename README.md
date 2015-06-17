#Lhttpd
[![Travis Build Status](https://travis-ci.org/loggerhead/lhttpd.svg)](https://travis-ci.org/loggerhead/lhttpd)

**NOTE**: Lhttpd is not thread-safe.

#Build and Install
##Dependency
You need [cmake](http://www.cmake.org/) and [make](http://www.gnu.org/software/make/) for build, and below libraries for compile.

* [libuv](https://github.com/libuv/libuv)

If you want run tests, you should run below command.

```shell
# Linux user run `sudo apt-get install python-dev` first
sudo pip install requests grequests
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
You can use everything in [lhttpd.h](https://github.com/loggerhead/lhttpd/blob/master/include/lhttpd.h).

#TODO
* [ ] 支持 `json`
* [ ] 支持 `redis`/`sqlite`
* [ ] 处理 `multipart/form-data`/`application/x-www-form-urlencoded`/`application/json`，参考 [四种常见的 POST 提交数据方式](https://www.imququ.com/post/four-ways-to-post-data-in-http.html#toc-2)、[Form-based File Upload in HTML](https://www.ietf.org/rfc/rfc1867.txt)
* [ ] 支持 `Transfer-Encoding`
* [ ] 支持 `gzip` 等压缩
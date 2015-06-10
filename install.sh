#!/bin/sh
# @Author: fz
# @Date:   2015-05-15 15:55:41
# @Last Modified by:   fz
# @Last Modified time: 2015-06-10 17:16:31

if [[ "$1" == "make" ]];then
    cmake . && make
elif [[ "$1" == "clean" ]];then
    rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile
    rm -rf liblhttpd.a test/*.dSYM
elif [[ "$1" == "test" ]];then
    if cmake . && make ;then
        cd test
        python runtests.py
    fi
fi
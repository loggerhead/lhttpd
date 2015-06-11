#!/bin/bash
# @Author: fz
# @Date:   2015-05-15 15:55:41
# @Last Modified by:   fz
# @Last Modified time: 2015-06-11 13:52:37

if [[ "$1" == "make" ]] || [[ -z $1 ]];then
    cmake . && make
elif [[ "$1" == "install" ]];then
    make install
elif [[ "$1" == "clean" ]];then
    cd test
    make clean
    rm -rf *.dSYM
    rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile
    cd ..
    make clean
    rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile
elif [[ "$1" == "test" ]];then
    cmake . && make && cd test && cmake . && make && python runtests.py
fi
#!/bin/bash
# @Author: fz
# @Date:   2015-05-15 15:55:41
# @Last Modified by:   fz
# @Last Modified time: 2015-06-10 22:03:49

if [[ "$1" == "make" ]];then
    cmake . && make
elif [[ "$1" == "clean" ]];then
    cd test
    make clean
    rm -rf *.dSYM
    rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile
    cd ..
    make clean
    rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile
elif [[ "$1" == "test" ]];then
    if cmake . && make ;then
        cd test && cmake . && make
        python runtests.py
    fi
fi
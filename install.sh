#!/bin/bash

if [[ -z $1 ]];then
    cmake . && make
    sudo make install
    ./install.sh test
elif [[ "$1" == "make" ]];then
    cmake . && make
elif [[ "$1" == "install" ]];then
    sudo make install
elif [[ "$1" == "clean" ]];then
    cd test
    make clean
    rm -f install_manifest.txt
    rm -rf *.dSYM
    rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile
    cd ..
    make clean
    rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile
elif [[ "$1" == "test" ]];then
    cd test && cmake . && make && python runtests.py
fi

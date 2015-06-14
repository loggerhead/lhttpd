#!/bin/bash

function _cmake_clean() {
    if [[ ! -z $1 ]]; then cd $1; fi

    make clean
    rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile

    if [[ ! -z $1 ]]; then cd ..; fi
}

function _clean() {
    _cmake_clean examples
    _cmake_clean test
    _cmake_clean

    rm -f install_manifest.txt
    cd test
    rm -f *.pyc
    rm -rf *.dSYM
    cd ..
}

function _make() {
    cmake . && make
}

function _install() {
    sudo make install
}

function _test() {
    cd test && cmake . && make && python runtests.py && cd ..
}

if [[ -z $1 ]];then
    _make
    _install
elif [[ "$1" == "make" ]];then
    _make
elif [[ "$1" == "install" ]];then
    _install
elif [[ "$1" == "clean" ]];then
    _clean
elif [[ "$1" == "test" ]];then
    _test
fi
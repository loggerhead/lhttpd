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
    rm -rf *.dSYM

    cd test
    rm -f *.log
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
    if cd test && _make; then
        python runtests.py
        cd ..
    fi
}

function _check() {
    TEST_EXEC=./test_webrouter

    if cd test && _make && [[ -x $TEST_EXEC ]]; then
        valgrind --leak-check=full --show-leak-kinds=all --dsymutil=yes --log-file=leak_check.log $TEST_EXEC &
        sleep 1
        siege -c100 -b -if siege_urls.txt 1> benchtest.log

        pkill -f valgrind
        siege -t2s -if siege_urls.txt &> /dev/null
        cd ..
    fi
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
elif [[ "$1" == "check" ]];then
    _check
fi
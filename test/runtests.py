#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author: fz
# @Date:   2015-05-16 22:28:38
# @Last Modified by:   fz
# @Last Modified time: 2015-06-05 19:39:12

import os
import json
import unittest
import requests
import subprocess

TEST_HOST = 'http://127.0.0.1:9999/'
NULL_OUT = open("/dev/null","wb")

def compile_test(filename):
    subprocess.call(["make", "all", 'FILENAME="%s"' % filename], stdout=NULL_OUT)

def run_test(filename, in_background=False):
    execname = os.path.splitext(filename)[0]
    args = ["./"+execname]
    if in_background:
        return subprocess.Popen(args)
    else:
        return subprocess.call(args)

class BaseLhttpdTestCase(unittest.TestCase, object):
    def runTest(self):
        testname = type(self).__name__
        if testname in BaseLhttpdTestCase.__name__:
            return

        compile_test(testname)
        return run_test(testname)

class test_httputil(BaseLhttpdTestCase):
    pass

class test_util(BaseLhttpdTestCase):
    pass

class test_httpserver(BaseLhttpdTestCase):
    def setUp(self):
        testname = type(self).__name__
        compile_test(testname)
        self.server_process = run_test(testname, in_background=True)

    def runTest(self):
        data = {'foo': 'I am foo', 'bar': 'I like bar'}

        r = requests.get(TEST_HOST)
        assert r.text == "hello, world"

        r = requests.post(TEST_HOST, data=data)
        assert r.text == "hello, world"

        r = requests.post(TEST_HOST, data=json.dumps(data))
        assert r.text == "hello, world"

        # send 10MB data
        r = requests.post(TEST_HOST, data='A' * (1024**3))
        assert r.text == "hello, world"

    def setTear(self):
        self.server_process.terminate()


if __name__ == "__main__":
    unittest.main()
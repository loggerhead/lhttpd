#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author: fz
# @Date:   2015-05-16 22:28:38
# @Last Modified by:   fz
# @Last Modified time: 2015-06-16 22:57:08

import os
import json
import time
import socket
import unittest
import requests
import grequests
import subprocess

TEST_PORT = 9998
TEST_HOST = '127.0.0.1'
TEST_URL = 'http://%s:%d' % (TEST_HOST, TEST_PORT)
TEST_DATA = 'A' * (1024**2)

NULL_OUT = open("/dev/null","wb")


def run_test(filename, in_background=False, args=[]):
    execname = os.path.splitext(filename)[0]
    execname = os.path.join(os.path.dirname(os.path.abspath(__file__)), execname)

    args = [execname] + [str(arg) for arg in args]

    if in_background:
        return subprocess.Popen(args, preexec_fn=os.setsid)
    else:
        return subprocess.call(args)

class BaseTestCase(unittest.TestCase, object):
    def runTest(self):
        testname = type(self).__name__
        if testname in BaseTestCase.__name__:
            return

        run_test(testname)

class BaseServerTestCase(unittest.TestCase, object):
    def setUp(self):
        testname = type(self).__name__
        if testname in BaseServerTestCase.__name__:
            return

        self.server_process = run_test(testname, in_background=True, args=[TEST_HOST, TEST_PORT])
        time.sleep(0.1)

    def tearDown(self):
        self.server_process.kill()


class test_httputil(BaseTestCase):
    pass

class test_util(BaseTestCase):
    pass

class test_httpserver(BaseServerTestCase):
    def runTest(self):
        EXPECT = 'F' * 100000

        rs = [
            grequests.get(TEST_URL + "/path/to/resouce"),
            grequests.post(TEST_URL, data=TEST_DATA),
        ]
        responses = grequests.map(rs)
        for response in responses:
            assert response.text == EXPECT

        status_code = 404
        r = requests.post(TEST_URL, data=str(status_code))
        assert r.status_code == status_code

class test_webrouter(BaseServerTestCase):
    def runTest(self):
        get = lambda tail: requests.get(TEST_URL + tail)
        post = lambda tail, data=None: requests.post(TEST_URL + tail, data=data)

        assert get("").text == "static"
        assert get("/").text == "static"
        assert get("/static").text == "static"
        assert get("/static/").text == "static"
        assert post("/static/test.html", data=TEST_DATA).status_code == 201
        assert get("/static/test.html").status_code == 404

        assert get("/x").status_code == 200
        assert get("/x/").status_code == 200
        assert get("/y/x").status_code == 200
        assert post("/x/y").status_code == 200
        assert get("/x/y").status_code == 404

        assert get("/int/1").status_code == 200
        assert get("/1/int").status_code == 200
        assert get("/int/测试").status_code == 404

        assert get("/path/to/good/end").status_code == 200
        assert post("/path/bad/").status_code == 200

        assert get("/above/1/test").text == "hello world"
        assert get("/above/1/bar/here").text == "hello world"
        assert get("/above/1/bar/here?q=value").text == "hello world"
        assert post("/below/0/bar/there").status_code == 200


if __name__ == "__main__":
    unittest.main()
#!/usr/bin/env python
#coding=utf-8

import time
import numpy as np
import math

def load_matrix_sub():
    rows = 640
    cols = 480

    mata = np.zeros((rows, cols), np.uint8)

    start = time.time()
    for i in range(rows):
        for j in range(cols):
            matx = mata[i:rows, j:cols]
    end = time.time()
    print "attach in %s seconds" % (end - start)

    start = time.time()
    for i in range(rows):
        for j in range(cols):
            matx = mata[i:rows, j:cols].copy()
    end = time.time()
    print "detach part in %s seconds" % (end - start)

    start = time.time()
    for i in range(rows):
        for j in range(cols):
            matx = mata.copy()
    end = time.time()
    print "clone in %s seconds" % (end - start)


def load_matrix_set():
    rows = 640
    cols = 480

    mata = np.zeros((rows, cols), np.uint8)

    start = time.time()
    for i in range(rows):
        for j in range(cols):
            white = np.ones((rows-i, cols-j), np.uint8)
            matx = mata.copy()
            matx[i:rows, j:cols] = white
    end = time.time()
    print "set in %s seconds" % (end - start)


def load_matrix_sum():
    rows = 640
    cols = 480

    mata = np.zeros((rows, cols), np.uint8)

    start = time.time()
    for i in range(rows):
        for j in range(cols):
            matx = mata
            matx[i:rows, j:cols].sum()
    end = time.time()
    print "summary in %s seconds" % (end - start)


load_matrix_sub()
load_matrix_set()
load_matrix_sum()

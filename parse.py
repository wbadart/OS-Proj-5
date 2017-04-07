#!/usr/bin/env python2

'''
' parse.py
'
' Take the results of benchmark.sh and turn them
' into pretty plots.
'
' Badart, Cat
' Badart, Will
' created: APR 2017
'''

import sys

def parse_line(line):
    params, data = line.split(',')[:4], line.split(':')[1:]
    params[0], params[1] = int(params[0]), int(params[1])
    data[1], data[3], data[5] = int(data[1]), int(data[3]), int(data[5])
    return params, data

def main():
    data = (parse_line(l.rstrip()) for l in open('results.csv', 'r'))
    print(next(data))

if __name__ == '__main__':
    main()


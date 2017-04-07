#!/afs/nd.edu/user15/pbui/pub/anaconda-2.3.0/bin/python

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

import os
import sys
import matplotlib as plt

from subprocess import check_output

def virtmem(npages, nframes, alg, prog):
    return check_output(['./virtmem', str(npages), str(nframes), alg, prog]
                        , stderr=open(os.devnull, 'w') ).split('\n')[-3]

def parse_result(line):
    return tuple(map(int, line.split(':')[1:][1::2]))

def main():
    PROGS, ALGS, N_FRAMES = ['focus', 'scan', 'sort']\
                          , ['rand', 'filo', 'custom']\
                          , xrange(10, 100, 10)
    data = []
    for p in PROGS:
        for a in ALGS:
            for n in N_FRAMES:
                data.append(parse_result(virtmem(100, n, a, p)))
    print data[12]

if __name__ == '__main__': main()


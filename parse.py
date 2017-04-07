#!/afs/nd.edu/user15/pbui/pub/anaconda-2.3.0/bin/python

'''
' parse.py
'
' Take the results of benchmark.sh and turn them
' into pretty plots.

' Record page faults, disk reads, and disk writes for each
' program, for each eviction algorithm, using fixed 100 pages,
' for each N frames from 3 to 100.
'
' Badart, Cat
' Badart, Will
' created: APR 2017
'''

import signal
import subprocess
import os
import matplotlib.pyplot as plt
import matplotlib.colors as colors

def virtmem(npages, nframes, alg, prog):
    return subprocess.check_output(
            [ './virtmem', str(npages), str(nframes), alg, prog ]
            , stderr=open(os.devnull, 'w') ).split('\n')[-3]

def parse_result(line):
    return tuple(map(int, line.split(':')[2::2]))

def timeout_handler(signum, frame):
    raise Exception('timeout')

def alg_prog_filter(alg, prog):
    return lambda d: d[0][1] == alg and d[0][2] == prog

def get_nreads(d):
    return d[1][1]

def get_nwrites(d):
    return d[1][2]

def main():
    signal.signal(signal.SIGALRM, timeout_handler)
    PROGS, ALGS, N_FRAMES = ['focus', 'scan', 'sort']\
                          , ['rand', 'filo', 'custom']\
                          , xrange(11, 100, 10)
    data = []
    for p in PROGS:
        for a in ALGS:
            for n in N_FRAMES:
                signal.alarm(10)
                print 'INFO: attempting args {} {} {} {}'.format(
                        100, n, a, p)
                try:
                    data.append(((n, a, p), parse_result(virtmem(99, n, a, p))))
                    signal.alarm(0)
                except subprocess.CalledProcessError as e:
                    print 'ERROR: Non-zero exit code. Possible SEGV'
                    data.append(((n, a, p), (0, 0, 0)))
                    signal.alarm(0)
                except Exception as e:
                    print 'ERROR: Timeout. Possible infinite loop'
                    data.append(((n, a, p), (0, 0, 0)))

    clrs = colors.cnames.values()
    fig_reads, fig_writes = plt.figure(), plt.figure()
    ax_reads, ax_writes   = fig_reads.add_subplot(111)\
                          , fig_writes.add_subplot(111)

    x, y_reads, y_writes = N_FRAMES, [], []
    for p in PROGS:
        for a in ALGS:
            target_data = (d for d in data if alg_prog_filter(a, p)(d))
            y_reads.append(map(get_nreads, target_data))
            y_writes.append(map(get_nwrites, target_data))

    for y in y_reads:
        y, c = y or [0] * len(x), clrs.pop()
        ax_reads.plot(x, y, color=c)
        ax_reads.scatter(x, y, color=c)
    for y in y_writes:
        y, c = y or [0] * len(x), clrs.pop()
        ax_writes.plot(x, y, color=c)
        ax_writes.scatter(x, y, color=c)

    plt.show()

if __name__ == '__main__': main()


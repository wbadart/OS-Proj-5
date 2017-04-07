#!/bin/sh

##
# benchmark.sh
#
# Run a variety of simulations using the
# virtual memory program.
#
# Record page faults, disk reads, and disk writes for each
# program, for each eviction algorithm, using fixed 100 pages,
# for each N frames from 3 to 100.
#
# Badart, Cat
# Badart, Will
# created: APR 2017
##

PROGS="focus scan sort"
ALGS="rand filo custom"
N_FRAMES=`seq 3 10 100`
N_PAGES=100

test -e ./virtmem || make > /dev/null 2> /dev/null

for prog in $PROGS; do
    for alg in $ALGS; do
        for n_frames in $N_FRAMES; do
            printf "$N_PAGES,$n_frames,$alg,$prog,"
            ./virtmem $N_PAGES $n_frames $alg $prog 2> /dev/null |
                grep "^RESULT" | tail -n 1
        done
    done
done


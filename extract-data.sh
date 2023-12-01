#!/usr/bin/bash

for n in 10 100 1000 10000 100000 1000000 10000000 100000000 1000000000; do
    for w in 1 10 100 1000 10000; do
	echo $n,$w,$(grep TOTAL nvidia-perf-$n-$w.txt | cut -d, -f2)
    done
done

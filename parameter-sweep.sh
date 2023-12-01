#!/usr/bin/bash

# invoke with -s option for sequential execution

# on NVIDIA RTX A6000 (banana)
# n = 10^9 w = 100 t = 35s

echo additional args: $*
date
for n in 10 100 1000 10000 100000 1000000 10000000 100000000 1000000000; do
    echo n = $n
    for w in 1 10 100 1000 10000; do
	echo n = $n, w = $w
	./build-nvidia/bin/integration $* -n $n -w $w > nvidia-perf-$n-$w.txt
	date
    done
done

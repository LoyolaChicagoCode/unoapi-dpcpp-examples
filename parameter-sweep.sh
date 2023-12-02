#!/usr/bin/bash

# invoke with -s option for sequential execution

# on NVIDIA RTX A6000 (banana)
# n = 10^9 t = 0.5s
# n = 10^10 t = 4s
# n = 10^11 t = 35s

echo additional args: $*
date
for n in 1000000000 10000000000 100000000000; do
    echo n = $n
    for g in 10 20 50 100 200 500 1000 2000 5000 10000 20000 50000 100000 200000 500000 1000000 2000000 5000000 10000000 20000000 50000000 100000000; do
	echo n = $n, g = $g
	./build-nvidia/bin/integration $* -n $n -g $g > nvidia-perf-$n-$g.txt
	date
    done
done

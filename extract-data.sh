#!/usr/bin/bash

# extracts the specified row from all raw performance data files
# (default: TOTAL)

row=${1:-TOTAL}
prefix=nvidia-perf-

for f in ${prefix}*.txt; do
    x=${f#$prefix}
    n=${x%-*}
    g=${x#*-}
    g=${g%.txt}
    echo $n,$g,$(grep $row $f | cut -d, -f2)
done

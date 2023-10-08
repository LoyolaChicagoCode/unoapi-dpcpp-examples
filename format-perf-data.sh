awk -F, 'NR>8 {printf "%.3f %s\n", $2/1000000000, $5}'

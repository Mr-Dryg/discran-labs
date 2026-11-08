#!/bin/bash
fail=0
for i in $(seq 1 200); do
    python3 gen.py $((RANDOM%8+1)) $((RANDOM%8+1)) $((RANDOM%6+1)) $i > in.txt
    ./sol < in.txt > o1.txt
    ./brute < in.txt > o2.txt
    if ! cmp -s o1.txt o2.txt; then
        echo "РАСХОЖДЕНИЕ, seed=$i"; cp in.txt bad.txt; fail=1; break
    fi
done
[ $fail -eq 0 ] && echo "OK: 200 малых тестов совпали"
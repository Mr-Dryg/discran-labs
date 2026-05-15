#!/bin/bash

echo "=== Building ==="
make bench
echo ""

run_test() {
    local size=$1
    local size_label=$2

    echo "=== Testing with text size: ${size_label} ==="

    ./gen.out $size > test_bench.txt

    ./kmp.out < test_bench.txt > res_kmp.txt
    echo "KMP:"
    cat res_kmp.txt | grep "time"

    ./main.out < test_bench.txt > res_main.txt
    echo "Suffix array:"
    cat res_main.txt | grep "time"

    echo ""

    rm test_bench.txt res_kmp.txt res_main.txt
}

run_test 10000 "10,000"
run_test 100000 "100,000"
run_test 1000000 "1,000,000"

echo "=== All tests completed ==="
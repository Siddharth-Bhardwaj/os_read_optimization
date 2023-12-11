#!/bin/bash

gcc run.c -o run
gcc run2.c -o run2
gcc syscallslseek.c -o syscallslseek
gcc fast.c -o fast #check if -lpthread flag is needed

chmod +x caching.sh
chmod +x syscalls.sh
chmod +x fast.sh

echo ""
echo "******************** PART 1: BASICS ********************"
echo ""

echo "WRITING to a test file with block_size = 256 and block_count = 512000"
./run test -w 256 512000
echo ""

echo "READING the test file with block_size = 256 and block_count = 512000"
./run test -r 256 512000
echo ""

echo "WRITING to a test file with block_size = 1024 and block_count = 1024000"
./run test -w 1024 1024000
echo ""

echo "READING the test file with block_size = 1024 and block_count = 1024000"
./run test -r 1024 1024000
echo ""

echo "TESTING negative case when bytes to be read are more than the file size"
./run test -r 1024 1024001
echo ""

echo ""
echo "******************** PART 2: MEASUREMENT ********************"
echo ""

echo "finding optimal block count and file size for block size = 64"
./run2 test 64 > /dev/null 2>&1 #loading cache
./run2 test 64
echo ""

echo "finding optimal block count and file size for block size = 128"
./run2 test 128
echo ""

echo "finding optimal block count and file size for block size = 256"
./run2 test 256
echo ""

echo "finding optimal block count and file size for block size = 512"
./run2 test 512
echo ""

echo "finding optimal block count and file size for block size = 1024"
output=$(./run2 test 1024)
echo "$output"
echo ""

let "blockcount=$(echo "$output" | grep -oP 'Optimal block count: \K\d+')"
echo "$blockcount will be used in Part 3 as the block count"
echo ""

echo ""
echo "******************** PART 3: RAW PERFORMANCE ********************"
echo ""

for ((i = 2; i <= 1024; i *= 2)); do
	echo "block size: $i"
	./run test -r "$i" "$blockcount" -mb
	echo ""
done
echo ""

echo ""
echo "******************** PART 4: CACHING ********************"
echo ""

./caching.sh
echo ""

echo ""
echo "******************** PART 5: SYSTEM CALLS ********************"
echo ""

./syscalls.sh
echo ""

echo ""
echo "******************** PART 6: RAW PERFORMANCE ********************"
echo ""
./fast.sh
echo ""
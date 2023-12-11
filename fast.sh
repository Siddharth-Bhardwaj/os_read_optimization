gcc run.c -o run
gcc fast.c -o fast

for ((i = 2; i <= 512; i *= 2)); do
	echo "*** BLOCK SIZE: $i ***"
	echo "creating test file with block_size = $i and block_count = 2048000"
	./run test -w "$i" 2048000
	let "file_size=(${i}*2048000)"
	echo "file size: $file_size bytes"
	echo ""

	echo "CLEARING CACHE"
	sh -c "/usr/bin/echo 3 > /proc/sys/vm/drop_caches"	
	echo ""

	echo "BEFORE CACHING reading test file with simple read from Part: 1 with block_size 1024 and block_count 2048000"
	./run test -r "$i" 2048000 -mb
	echo ""

	echo "AFTER CACHING reading test file with simple read from Part: 1 with block_size 1024 and block_count 2048000"
	./run test -r "$i" 2048000 -mb
	echo ""

	echo "CLEARING CACHE"
	sh -c "/usr/bin/echo 3 > /proc/sys/vm/drop_caches"	
	echo ""

	echo "BEFORE CACHING: reading test file with fast.c, i.e., optimal block size and multithreading"
	./fast test
	echo ""

	echo "AFTER CACHING: reading test file with fast.c, i.e., optimal block size and multithreading"
	./fast test
	echo ""

	echo ""
done
gcc run.c -o run
echo "creating/writing to file 'test' with block_size = 1024 and block_count = 204800"
./run test -w 1024 204800
echo ""

block_sizes=(2 4 8 16 32 64 128 256 512 1024)

for block_size in "${block_sizes[@]}"; do
	echo "CLEARING CACHE"
	sh -c "/usr/bin/echo 3 > /proc/sys/vm/drop_caches"	
	echo ""
	echo "block size: ${block_size}"
	let "block_count=(1024/${block_size})*204800"
	echo "reading file with empty cache"
	./run test -r "${block_size}" "${block_count}" -mb
	echo ""
	echo "reading file again without clearing cache"
	./run test -r "${block_size}" "${block_count}" -mb
	echo ""
done
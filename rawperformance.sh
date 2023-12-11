gcc run.c -o run

echo "writing test file with block_size = 1024 and block_count = 2097152"
./run test -w 1024 2097152

echo ""

for ((i = 2; i <= 1024; i *= 2)); do
	echo "block size: $i"
	./run test -r "$i" 2097152 -mb
	echo ""
done
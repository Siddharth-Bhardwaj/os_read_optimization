gcc run.c -o run
gcc syscallslseek.c -o syscallslseek

for ((i = 2; i <= 8192; i *= 2)); do
	echo "creating syscalls.txt with block_size = 1024 and block_count = $i"
	./run syscalls.txt -w 1024 "$i"
	let "file_size=(${i}*1024)"
	echo "file size: $file_size bytes"
	echo ""

	echo "reading syscalls.txt with block_size = 1"
	./run syscalls.txt -r 1 "$file_size" -mb -b
	echo ""	

	echo "lseek on syscalls.txt with block_size = 1"
	./syscallslseek syscalls.txt
	echo ""

	echo "*****"
	echo ""
done

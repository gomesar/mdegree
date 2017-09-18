#!/bin/bash
if [ ! -d ./bin/ ];then
	echo "Creating bin folder"
	mkdir -p bin
fi
gcc global.c -o bin/glob
gcc semi-global.c -o bin/sglob
gcc local.c -o bin/loc
gcc L1Q5.c -o bin/l1q5
gcc L1Q6.c -o bin/l1q6

if [ ! -z "$1" ]; then
	if [ $1 == "test" ]; then
		echo "Test enabled"
		./_test.sh
	fi
fi


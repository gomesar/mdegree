#!/bin/bash
gcc global.c -o glob
gcc semi-global.c -o sglob
gcc local.c -o loc
gcc custom1.c -o alg1
gcc custom2.c -o alg2

if [ ! -z "$1" ]; then
	if [ $1 == "test" ]; then
		echo "Test enabled"
		./_test.sh
	fi
fi


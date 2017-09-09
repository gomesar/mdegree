#!/bin/bash
gcc global.c -o glob
gcc semi-global.c -o sglob
gcc custom1.c -o alg1

if [ $1 == "test" ]; then
	./_test.sh
fi


#!/bin/bash
gcc -Wall -Werror -O2 -g3 src/main.c -o src/main

# ./main < test.txt
src/main < ./tests/input/input_pub.txt
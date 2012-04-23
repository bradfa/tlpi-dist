#!/bin/sh
#
# build.sh
#
# Build script for -rpath linker option example
# 
set -v

ROOT="$PWD"

# Enabling the following means the executable rpath with be
# built as a DT_RUNPATH entry rather than a DT_RPATH entry

#DTAGS="-Wl,--enable-new-dtags"

cd $ROOT/d2
gcc -g -c -fPIC -Wall modx2.c
gcc -g -shared -o libx2.so modx2.o

cd $ROOT/d1
gcc -g -c -fPIC -Wall modx1.c
gcc -g -shared  -o libx1.so modx1.o $DTAGS \
    	-Wl,-rpath,$ROOT/d2 -L$ROOT/d2 -lx2 

cd $ROOT
gcc -g -Wall -o prog prog.c $DTAGS \
    	-Wl,-rpath,$ROOT/d1 -L$ROOT/d1 -lx1 
	
objdump -p prog | grep PATH
objdump -p d1/libx1.so | grep PATH
ldd prog

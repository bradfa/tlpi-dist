#!/bin/sh
#
# build-rpath-link.sh
#
# Build script for -rpath-link linker option example
# 
set -v

ROOT=$PWD

cd $ROOT/d2
gcc -g -c -fPIC -Wall modx2.c
gcc -g -shared -o libx2.so modx2.o

cd $ROOT/d1
gcc -g -c -fPIC -Wall modx1.c
gcc -g -shared  -o libx1.so modx1.o \
	-L$ROOT/d2 -lx2 

cd $ROOT
gcc -g -Wall -o prog prog.c \
    	-Wl,-rpath,$ROOT/d1 -L$ROOT/d1 -lx1 \
    	-Wl,-rpath-link,$ROOT/d2 
	
objdump --all-headers prog | grep RPATH
objdump --all-headers d1/libx1.so | grep RPATH
ldd prog

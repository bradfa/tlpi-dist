#!/bin/sh

set -v

gcc -g -shared -fPIC -o libfoo1.so foo1.c
gcc -g -Wl,-Bsymbolic -shared -fPIC -o libfoo2.so foo2.c
gcc -g -Wl,-Bsymbolic -Wl,-allow-shlib-undefined \
    	-shared -fPIC -o libfoo3.so foo3.c

gcc -g -o prog prog.c -L. -lfoo1 -lfoo2 -lfoo3

LD_LIBRARY_PATH=. ./prog

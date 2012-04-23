# Makefile to build library used by all programs
#
# This make file relies on the assumption that each C file in this
# directory belongs in the library
#
# This makefile is very simple so that every version of make 
# should be able to handle it
#
include ../Makefile.inc

# The library build is "brute force" -- we don't bother with 
# dependency checking.

allgen :
	sh Build_ename.sh > ename.c.inc
	echo 1>&2 "ename.c.inc built"
	${CC} -c -g ${CFLAGS} *.c
	${RM} ${TLPI_LIB}
	${AR} rs ${TLPI_LIB} *.o

clean :
	${RM} *.o ename.c.inc ${TLPI_LIB}


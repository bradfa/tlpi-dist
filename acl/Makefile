include ../Makefile.inc

GEN_EXE = 

LINUX_EXE = acl_update acl_view

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

acl_update : acl_update.o
	${CC} -o $@ acl_update.o ${CFLAGS} ${LDLIBS} ${LINUX_LIBACL}

acl_view : acl_view.o
	${CC} -o $@ acl_view.o ${CFLAGS} ${LDLIBS} ${LINUX_LIBACL}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

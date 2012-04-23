include ../Makefile.inc

GEN_EXE = psem_getvalue psem_create psem_post psem_unlink \
	  psem_timedwait psem_trywait psem_wait thread_incr_psem

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBRT}
	# All of the programs in this directory need the 
	# realtime library, librt.

thread_incr_psem: thread_incr_psem.o
	${CC} -o $@ thread_incr_psem.o \
		${CFLAGS} ${IMPL_THREAD_FLAGS} ${LDLIBS}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

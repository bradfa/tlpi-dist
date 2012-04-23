include ../Makefile.inc

GEN_EXE = mq_notify_sig mq_notify_sigwaitinfo mq_notify_thread \
	  pmsg_create pmsg_getattr pmsg_receive pmsg_send pmsg_unlink

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBRT}
	# All of the programs in this directory need the 
	# realtime library, librt.

mq_notify_thread: mq_notify_thread.c
	${CC} -o $@ ${IMPL_THREAD_FLAGS} mq_notify_thread.c \
	    		${CFLAGS} ${LDLIBS}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

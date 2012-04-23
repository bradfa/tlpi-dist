include ../Makefile.inc

GEN_EXE = demo_timerfd ptmr_null_evp ptmr_sigev_signal ptmr_sigev_thread \
	real_timer t_nanosleep timed_read

LINUX_EXE = t_clock_nanosleep demo_timerfd

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

demo_timerfd: demo_timerfd.o
	${CC} -o $@ demo_timerfd.o ${CFLAGS} ${LDLIBS} ${LINUX_LIBRT}

ptmr_null_evp: ptmr_null_evp.o
	${CC} -o $@ ptmr_null_evp.o ${CFLAGS} ${LDLIBS} ${LINUX_LIBRT}

ptmr_sigev_signal: ptmr_sigev_signal.o
	${CC} -o $@ ptmr_sigev_signal.o ${CFLAGS} ${LDLIBS} ${LINUX_LIBRT}

ptmr_sigev_thread: ptmr_sigev_thread.c
	${CC} -o $@ ${IMPL_THREAD_FLAGS} ptmr_sigev_thread.c \
	    		${CFLAGS} ${LDLIBS} ${LINUX_LIBRT}

t_clock_nanosleep: t_clock_nanosleep.o
	${CC} -o $@ t_clock_nanosleep.o ${CFLAGS} ${LDLIBS} ${LINUX_LIBRT}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

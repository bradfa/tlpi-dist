include ../Makefile.inc

GEN_EXE = demo_sched_fifo sched_set sched_view t_setpriority 

LINUX_EXE = t_sched_setaffinity t_sched_getaffinity

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

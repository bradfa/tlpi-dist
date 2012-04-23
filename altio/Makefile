include ../Makefile.inc

GEN_EXE = demo_sigio poll_pipes select_mq self_pipe t_select

LINUX_EXE = epoll_input

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

include ../Makefile.inc

GEN_EXE = syscall_speed

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

syscall_speed : syscall_speed.o
	${CC} -o $@ syscall_speed.o ${CFLAGS} ${LDLIBS}

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

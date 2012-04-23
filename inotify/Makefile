include ../Makefile.inc

GEN_EXE =

LINUX_EXE = demo_inotify dnotify

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allftw : ${FTW_EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

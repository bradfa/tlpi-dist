include ../Makefile.inc

GEN_EXE = copy mix23_linebuff mix23io write_bytes

LINUX_EXE = direct_read

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

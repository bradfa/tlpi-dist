include ../Makefile.inc

GEN_EXE = pshm_create pshm_read pshm_write pshm_unlink

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

CFLAGS = ${IMPL_CFLAGS} ${LINUX_LIBRT}
	# On Linux, all of the programs in this directory need the 
	# realtime library, librt.

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

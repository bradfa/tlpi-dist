include ../Makefile.inc

GEN_EXE = 

LINUX_EXE = check_password_caps

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

demo_caps: demo_caps.o
	${CC} -o $@ demo_caps.o ${CFLAGS} ${LDLIBS} ${LINUX_LIBCAP}

check_password_caps: check_password_caps.o
	${CC} -o $@ check_password_caps.o ${CFLAGS} ${LDLIBS} \
			${LINUX_LIBCAP} ${LINUX_LIBCRYPT}

${EXE} : ${LPLIB}		# True as a rough approximation

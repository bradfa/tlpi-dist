# Makefile to build all programs in all subdirectories
#
# DIRS is a list of all subdirectories containing makefiles
# (The library directory is first so that the library gets built first)
#

DIRS = 	lib \
    	acl altio \
	cap \
	daemons dirs_links \
	fileio filelock files filesys getopt \
	inotify \
	loginacct \
	memalloc \
	mmap \
	pgsjc pipes pmsg \
	proc proccred procexec procpri procres \
	progconc \
	psem pshm pty \
	shlibs \
	signals sockets \
	svipc svmsg svsem svshm \
	sysinfo \
	threads time timers tty \
	users_groups \
	vmem \
	xattr

# Dummy targets for building and clobbering everything in all subdirectories

all: 	
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE}) ; done

allgen: 
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} allgen) ; done

clean: 
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} clean) ; done

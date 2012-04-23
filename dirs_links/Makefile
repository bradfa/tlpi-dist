include ../Makefile.inc

GEN_EXE = bad_symlink list_files list_files_readdir_r \
	t_dirbasename t_unlink view_symlink 

FTW_EXE = nftw_dir_tree file_type_stats

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE} ${FTW_EXE}

all : ${EXE}

allftw : ${FTW_EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${LPLIB}		# True as a rough approximation

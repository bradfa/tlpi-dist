# Makefile.inc - common definitions used by all makefiles

TLPI_DIR = ..
TLPI_LIB = ${TLPI_DIR}/libtlpi.a
TLPI_INCL_DIR = ${TLPI_DIR}/lib

LINUX_LIBRT = -lrt
LINUX_LIBDL = -ldl
LINUX_LIBACL = -lacl
LINUX_LIBCRYPT = -lcrypt
LINUX_LIBCAP = -lcap

# "-Wextra" is a more descriptive synonym for "-W", but only
# available in more recent gcc versions

IMPL_CFLAGS = -std=c99 -D_XOPEN_SOURCE=600 \
		-g -I${TLPI_INCL_DIR} \
		-pedantic \
		-Wall \
		-W \
		-Wpointer-arith \
		-Wmissing-prototypes \
		-Wno-sign-compare \
		-Wno-unused-parameter \
		-Wno-format-y2k \
		-Wno-long-long
		
CFLAGS = ${IMPL_CFLAGS}

IMPL_THREAD_FLAGS = -pthread

IMPL_LDLIBS = ${TLPI_LIB} -lm

LDLIBS = ${IMPL_LDLIBS}

RM = rm -f

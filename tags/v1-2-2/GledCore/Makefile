# $Header$

LIB_SET_NAME	= GledCore
LIB_SET_ID	= 1
REQUIRES_LIB_SETS =

# All this dirs expanded in make_base.inc into <type>_SRCS and _HDRS
# Can add your own before ... or after (are += -ed into the vars)

BLESSED_DIRS	= Glasses
BASE_DIRS	= Ephra Stones Gled Net Tvor

DICT_DIRS	= Glasses Stones Gled Ephra
VIEW_DIRS	= GledView Eye FL RnrBase
VIEW_FL_DIRS	= FL

CPPFLAGS	=-I/usr/kerberos/include
BASE_LD_LIBS	=-lssl

MAINSRCS := saturn.cxx gled.cxx
MAINOBJS := $(MAINSRCS:.cxx=.o)
MAINEXES := $(addprefix bin/, $(MAINSRCS:.cxx=))

LIBSET_TARGETS := ${MAINEXES}

include ${GLEDSYS}/make/make_base.inc

bin/gled: gled.o ${BASE_LIB_NAME} ${VIEW_LIB_NAME}
	${LD} -o $@ ${LDFLAGS} ${CXXFLAGS} gled.o \
	-Llib -l${LIB_SET_NAME} -l${LIB_SET_NAME}_View \
	${ROOTLIBS} ${SYSLIBS} ${GUILIBS} ${ROOTGLIBS}

bin/saturn: saturn.o ${BASE_LIB_NAME}
	${LD} -o $@ ${LDFLAGS} ${CXXFLAGS} saturn.o \
	-Llib -l${LIB_SET_NAME} \
	${ROOTLIBS} ${SYSLIBS}

.PHONY: libset_clean libset_distclean

libset_clean:
	@rm -f ${MAINOBJS}
libset_distclean:
	@rm -f ${MAINEXES}
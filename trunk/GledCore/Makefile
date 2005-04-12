# $Header$

LIB_SET_NAME	= GledCore
LIB_SET_ID	= 1
REQUIRES_LIB_SETS =

REGEXP_LIB_BASE  := GledCore_pcre
REGEXP_LIB       := lib/lib${REGEXP_LIB_BASE}.so

# All this dirs expanded in make_base.inc into <type>_SRCS and _HDRS
# Can add your own before ... or after (are += -ed into the vars)

BLESSED_DIRS	= Glasses
BASE_DIRS	= Ephra Stones Gled Net Tvor

DICT_DIRS	= Glasses Stones Gled Ephra
VIEW_DIRS	= GledView Eye FL RnrBase
VIEW_FL_DIRS	= FL

CPPFLAGS	=-I/usr/kerberos/include
BASE_LD_LIBS	=-lssl -Llib -l${REGEXP_LIB_BASE}

MAINSRCS := saturn.cxx gled.cxx
MAINOBJS := $(MAINSRCS:.cxx=.o)
MAINEXES := bin/saturn bin/gled

LIBSET_PREREQS := ${REGEXP_LIB}
LIBSET_TARGETS := local_targets

include ${GLEDSYS}/make/make_base.inc

local_targets:: ${REGEXP_LIB} bin/saturn
ifndef SKIP_VIEW_LIBS
local_targets:: bin/gled
endif

gled_core_targets: ${LOCAL_TARGETS}

bin/saturn: saturn.o ${BASE_LIB_NAME}
	${LD} -o $@ ${LDFLAGS} ${CXXFLAGS} saturn.o \
	-Llib -l${LIB_SET_NAME} \
	${ROOTLIBS} ${SYSLIBS}

bin/gled: gled.o ${BASE_LIB_NAME} ${VIEW_LIB_NAME}
	${LD} -o $@ ${LDFLAGS} ${CXXFLAGS} gled.o \
	-Llib -l${LIB_SET_NAME} -l${LIB_SET_NAME}_View -l${REGEXP_LIB_BASE} \
	${ROOTLIBS} ${SYSLIBS} ${GUILIBS} ${ROOTGLIBS}

${REGEXP_LIB}:
	${MAKE} -C RegExp

clean::
	@rm -f ${MAINOBJS}
	${MAKE} -C RegExp clean

distclean::
	@rm -f ${MAINEXES}
	${MAKE} -C RegExp distclean

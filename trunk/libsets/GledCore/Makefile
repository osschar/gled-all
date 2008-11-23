# $Id$

LIB_SET_NAME	= GledCore
LIB_SET_ID	= 1
REQUIRES_LIB_SETS =

# All this dirs expanded in make_base.inc into <type>_SRCS and _HDRS
# Can add your own before ... or after (are += -ed into the vars)

BLESSED_DIRS	= Glasses
BASE_DIRS	= Ephra Stones Gled Eye RnrBase
DICT_DIRS	= Glasses Stones Gled Ephra

VIEW_DIRS	= GledView FL
VIEW_FL_DIRS	= FL

BASE_LD_LIBS	=-lssl -Llib

MAINSRCS := saturn.cxx gled.cxx
MAINOBJS := $(MAINSRCS:.cxx=.o)
MAINEXES := bin/saturn bin/gled

PUPILS_LIB      = lib/libGledCore_Pupils.${LDEXT}
PUPILS_HDRS    := $(wildcard Pupils/*.h)
PUPILS_SRCS    := $(wildcard Pupils/*.cxx)
PUPILS_OBJS    := $(addsuffix .o ,$(basename ${PUPILS_SRCS}))

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

CPPFLAGS +=-I/usr/kerberos/include

LIBSET_TARGETS += bin/saturn

ifndef SKIP_VIEW_LIBS
  LIBSET_TARGETS   += bin/gled
  ifndef SKIP_RNR_LIBS
    ALL_SRCS       += ${PUPILS_SRCS}
    ALL_HDRS       += ${PUPILS_HDRS}
    ALL_OBJS       += ${PUPILS_OBJS}
    ALL_DEPS       += $(PUPILS_OBJS:.o=.d)
    LIBSET_TARGETS += ${PUPILS_LIB}
  endif
endif

bin/saturn: saturn.o ${BASE_LIB_NAME}
	${ECHO} -X- Linking $@
	${MUTE} ${LD} -o $@ ${LDFLAGS} ${CXXFLAGS} saturn.o \
	-Llib -l${LIB_SET_NAME} ${ROOTLIBS} ${SYSLIBS}

bin/gled: gled.o ${BASE_LIB_NAME} ${VIEW_LIB_NAME}
	${ECHO} -X- Linking $@
	${MUTE} ${LD} -o $@ ${LDFLAGS} ${CXXFLAGS} gled.o \
	-Llib -l${LIB_SET_NAME} -l${LIB_SET_NAME}_View \
	${ROOTLIBS} ${SYSLIBS} ${GUILIBS} ${ROOTGLIBS}


${PUPILS_LIB}: ${PUPILS_OBJS}
	${ECHO} -X- Linking $@
	${MUTE} ${LD} ${LDSHAREDOPT} -o $@ ${LDFLAGS} ${CXXFLAGS} ${PUPILS_OBJS}

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

install::
ifeq (${GLED_SYSTEM},Darwin)
	fltk-config --post ${GLEDSYS}/bin/gled
endif

clean::
	@rm -f ${MAINOBJS}

distclean::
	@rm -f ${MAINEXES}

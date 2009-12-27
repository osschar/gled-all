# $Id$
LIB_SET_NAME	:= Var1
LIB_SET_ID	:= 33
REQUIRES_LIB_SETS := GledCore Geom1 GTS Numerica Audio1

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones Var1 

DICT_DIRS	:= Glasses Stones Var1

VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=

LIBSET_CPPFLAGS = $(shell pkg-config --cflags glib-2.0)
BASE_LD_LIBS    = -lOpcode

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

LIBSET_PREREQS += lib/libOpcode.${LDEXT}
#LIBSET_TARGETS += 

# user targets go here; enumerate them in LIBSET_PREREQS/TARGETS

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

lib/libOpcode.${LDEXT}:
	${MAKE} -C Opcode

clean::
	${MAKE} -C Opcode clean

distclean::
	${MAKE} -C Opcode distclean

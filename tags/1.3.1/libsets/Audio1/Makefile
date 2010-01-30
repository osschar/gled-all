# $Id$
LIB_SET_NAME	:= Audio1
LIB_SET_ID	:= 4
REQUIRES_LIB_SETS := GledCore # add

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones Audio1 

DICT_DIRS	:= Glasses Stones

VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

ifeq (${GLED_SYSTEM},Darwin)
  BASE_LD_LIBS := -framework OpenAL
else
  BASE_LD_LIBS := -lopenal
endif
BASE_LD_LIBS += -lalut -lvorbisfile

LIBSET_TARGETS := # define additional targets (libraires built by default)

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

# $Header$
LIB_SET_NAME	:= TADemo
LIB_SET_ID	:= 32
REQUIRES_LIB_SETS := GledCore # add

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones TADemo 

DICT_DIRS	:= Glasses Stones TADemo

VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

LIBSET_TARGETS := # define additional targets (libraires built by default)

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

# $Id$

LIB_SET_NAME	:= Numerica
LIB_SET_ID	:= 3
REQUIRES_LIB_SETS := GledCore

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones Numerica 

DICT_DIRS	:= Glasses Stones Numerica
VIEW_FL_DIRS	:=
VIEW_DIRS	:=

BASE_LD_LIBS    := -lgsl

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

LIBSET_TARGETS := # define additional targets (libraires built by default)

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

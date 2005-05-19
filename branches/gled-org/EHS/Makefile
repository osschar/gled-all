# $Header$
LIB_SET_NAME	:= EHS
LIB_SET_ID	:= 17
REQUIRES_LIB_SETS := GledCore # add

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones EHS 

DICT_DIRS	:= Glasses Stones EHS

VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=

BASE_LD_LIBS	:= -lehs

##### Targets #####

LIBSET_TARGETS := # define additional targets (libraires built by default)

include ${GLEDSYS}/make/make_base.inc

# user targets go here; enumerate them in LIBSET_TARGETS

# clean and distclean defined as '::' rules in make_base.inc
clean::
distclean::

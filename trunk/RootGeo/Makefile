# $Header$
LIB_SET_NAME	:= RootGeo
LIB_SET_ID	:= 64
REQUIRES_LIB_SETS := GledCore # add

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones RootGeo 

DICT_DIRS	:= Glasses Stones RootGeo

VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=
BASE_LD_LIBS    = -L${ROOTSYS}/lib -lGeom -lRGL -lX3d

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

LIBSET_TARGETS := # define additional targets (libraires built by default)

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

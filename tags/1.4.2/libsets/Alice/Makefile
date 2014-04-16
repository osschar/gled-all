# $Header$

LIB_SET_NAME	  := Alice
LIB_SET_ID	  := 65
REQUIRES_LIB_SETS := GledCore Geom1 RootGeo

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones Alice 

DICT_DIRS	:= Glasses Stones

VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=
BASE_LD_LIBS    = ${ROOT_LD_DIR} -lGeom -lEG -lTreePlayer -lGed -lFTGL -lRGL -lEve

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

LIBSET_TARGETS := # define additional targets (libraires built by default)

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

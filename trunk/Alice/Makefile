# $Header$

LIB_SET_NAME	  := Alice
LIB_SET_ID	  := 65
REQUIRES_LIB_SETS := GledCore Geom1 RootGeo

# ALICE-stuff that must come from the environment:
# ALICE_ROOT=/home/aljam/alice/pro
# ALICE_TARGET=linuxDEBUG
# ALICE_TARGET=linux

ALICE_LIBS = -lESD -lSTEER -lCONTAINERS

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones Alice 

DICT_DIRS	:= Glasses Stones
CPPFLAGS	:=-I${ALICE_ROOT}/include -I${ALICE_ROOT}/TPC -I${ALICE_ROOT}/ITS -I${ALICE_ROOT}/TOF -I${ALICE_ROOT}/CONTAINERS

VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=
BASE_LD_LIBS    = -L${ROOTSYS}/lib -lGeom -lEG -lVMC -lTreePlayer \
		-L${ALICE_ROOT}/lib/tgt_${ALICE_TARGET} ${ALICE_LIBS}

##### Targets #####

LIBSET_TARGETS := # define additional targets (libraires built by default)

include ${GLEDSYS}/make/make_base.inc

# user targets go here; enumerate them in LIBSET_TARGETS

# clean and distclean defined as '::' rules in make_base.inc
clean::
distclean::

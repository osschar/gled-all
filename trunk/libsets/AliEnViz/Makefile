# $Header$
LIB_SET_NAME	:= AliEnViz
LIB_SET_ID	:= 66
REQUIRES_LIB_SETS := GledCore Geom1 Var1

#export MLCLIENT_BASE := AliEnViz_mlclient
#export MLCLIENT_LIB  := lib/lib${MLCLIENT_BASE}.${LDEXT}
#export MLCLIENT_BIN  := bin/ml_client

# Nice, friendly woozies that don't need special treatment
BLESSED_DIRS	:= Glasses
BASE_DIRS	:= Stones AliEnViz 

DICT_DIRS	:= Glasses Stones AliEnViz

VIEW_DIRS	:=
VIEW_FL_DIRS	:=

BASE_LD_LIBS    = -Llib #-l${MLCLIENT_BASE}  # -L${ROOTSYS}/lib -lProof

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

LIBSET_PREREQS += # ${MLCLIENT_LIB} ${MLCLIENT_BIN}
LIBSET_TARGETS += # define additional targets (libraires built by default)

# user targets go here; enumerate them in LIBSET_TARGETS

#${MLCLIENT_LIB} ${MLCLIENT_BIN}:
#	${MAKE} -C MLClient

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
#	${MAKE} -C MLClient clean

distclean::
#	${MAKE} -C MLClient distclean

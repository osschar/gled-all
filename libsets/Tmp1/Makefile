# $Header$
LIB_SET_NAME	:= Tmp1
LIB_SET_ID	:= 128
REQUIRES_LIB_SETS := GledCore Geom1 GledGTS # add

# Directories containing glasses - full Gled enabled classes.
BLESSED_DIRS	:= Glasses
# Directories to be included in base library.
BASE_DIRS	:= Stones Tmp1 

# Directories for which dictionaries will be generates.
DICT_DIRS	:= Glasses Stones Tmp1

# Directories to go into the View / GUI library.
VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=

LIB_WAC_DUMP_GLED := lib/libWacDumpGled.a
ifneq ($(wildcard ${LIB_WAC_DUMP_GLED}),)
BASE_LD_LIBS    = ${LIB_WAC_DUMP_GLED}
Glasses/TabletReader.o: CPPFLAGS += -DHAS_WAC_GLED
endif

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

# LIBSET_PREREQS +=
# LIBSET_TARGETS +=

# user targets go here; enumerate them in LIBSET_PREREQS/TARGETS

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

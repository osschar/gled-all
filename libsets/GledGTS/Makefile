# $Id$

LIB_SET_NAME	  := GledGTS
LIB_SET_ID	  := 16
REQUIRES_LIB_SETS := GledCore Numerica
REQUIRES_EXTERNAL := gts


BLESSED_DIRS	= Glasses
BASE_DIRS	= Stones GTS

DICT_DIRS	= Glasses Stones
VIEW_DIRS	= FL
VIEW_FL_DIRS	= FL

LIBSET_CPPFLAGS	= $(shell pkg-config --cflags glib-2.0)
BASE_LD_LIBS	= $(shell pkg-config --libs   glib-2.0) -lgts

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

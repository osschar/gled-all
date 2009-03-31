# $Id$
LIB_SET_NAME	= GTS
LIB_SET_ID	= 16
REQUIRES_LIB_SETS = GledCore # add

BLESSED_DIRS	= Glasses
BASE_DIRS	= Stones GTS

DICT_DIRS	= Glasses Stones
VIEW_DIRS	= FL
VIEW_FL_DIRS	= FL

LIBSET_CPPFLAGS	= $(shell pkg-config --cflags-only-I glib-2.0)
BASE_LD_LIBS	= $(shell pkg-config --libs-only-l glib-2.0) -lgts

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

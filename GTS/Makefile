# $Header$
LIB_SET_NAME	= GTS
LIB_SET_ID	= 16
REQUIRES_LIB_SETS = GledCore # add

BLESSED_DIRS	= Glasses
BASE_DIRS	= Stones GTS

DICT_DIRS	= Glasses Stones
VIEW_DIRS	= FL
VIEW_FL_DIRS	= FL

BASE_LD_LIBS	= -lglib -lgts

ifdef GLIB_CONFIG
CPPFLAGS	= $(shell ${GLIB_CONFIG} --cflags)
BASE_LD_LIBS	= $(shell ${GLIB_CONFIG} --libs) -lgts
else
CPPFLAGS        = -I/usr/include/glib-1.2 -I/usr/lib/glib/include
BASE_LD_LIBS	= -lglib -lgts
endif

include ${GLEDSYS}/make/make_base.inc

clean::
distclean::

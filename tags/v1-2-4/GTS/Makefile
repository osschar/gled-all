# $Header$
LIB_SET_NAME	= GTS
LIB_SET_ID	= 16
REQUIRES_LIB_SETS = GledCore # add

BLESSED_DIRS	= Glasses
BASE_DIRS	= Stones GTS

DICT_DIRS	= Glasses Stones
VIEW_DIRS	= FL
VIEW_FL_DIRS	= FL

ifndef GLIB_CONFIG
GLIB_CONFIG    := glib-config
endif

CPPFLAGS	= $(shell ${GLIB_CONFIG} --cflags)
BASE_LD_LIBS	= $(shell ${GLIB_CONFIG} --libs) -lgts

include ${GLEDSYS}/make/make_base.inc

clean::
distclean::

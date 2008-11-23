# $Id$
LIB_SET_NAME	= Geom1
LIB_SET_ID	= 2
REQUIRES_LIB_SETS = GledCore # add

BLESSED_DIRS	= Glasses
BASE_DIRS	= Stones Tvor ACTC GForge

DICT_DIRS	= Glasses Stones
VIEW_DIRS	= FL
VIEW_FL_DIRS	= FL

# libIL used to be linked against image libs (on most systems).
# The latest version (1.7.2) is not, also whre it was before.
# So ... until this is solved, I add the libs here.
# BASE_LD_LIBS    = -lIL -lILU
BASE_LD_LIBS    = -lIL -lILU

GFORGE_EXE      = bin/gforge

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

LIBSET_TARGETS := ${GFORGE_EXE}

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

${GFORGE_EXE}:
	${MAKE} -C GForge

clean::
	${MAKE} -C GForge clean

distclean::
	${MAKE} -C GForge distclean

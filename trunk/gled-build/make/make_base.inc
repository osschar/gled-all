# $Header$

##########################################################################
#
# Base make-include file for Gled ... and any add-on libsets
#
##########################################################################

all:	default

include ${GLEDSYS}/make/make_config.inc
include ${GLEDSYS}/make/make_defs.inc
include ${GLEDSYS}/make/make_rnr.inc

# include all make_*.inc files ...
ls_make_inc := $(wildcard make_*.inc)
ifneq (${ls_make_inc},)
include make_*.inc
endif

LIBSET_DIR	= ${GLEDSYS}/${LIB_SET_NAME}
ABOVE_BASE_LIBS	= $(addprefix -l, ${REQUIRES_LIB_SETS})
ABOVE_VIEW_LIBS	= $(addsuffix _View, $(addprefix -l, ${REQUIRES_LIB_SETS}))

BUILD_INSTALL_DIRS += perllib cfg lib-exec
INSTALL_DIRS	   += lib bin macros cfg perllib

##########################################################################
# Sources ... different levels of processing
##########################################################################

# Blessed classes
BLESSED_HDRS +=	$(foreach dir,${BLESSED_DIRS},$(wildcard ${dir}/*.h))
BLESSED_SRCS +=	$(foreach dir,${BLESSED_DIRS},$(wildcard ${dir}/*.cxx))
BLESSED_OBJS +=	$(addsuffix .o ,$(basename ${BLESSED_SRCS}))
BLESSED_H7   +=	$(addsuffix .h7,$(basename ${BLESSED_HDRS}))
BLESSED_C7   +=	$(addsuffix .c7,$(basename ${BLESSED_HDRS}))
.SECONDARY: ${BLESSED_H7} ${BLESSED_C7}

# Base classes
BASE_HDRS +=	$(foreach dir,${BASE_DIRS},$(wildcard ${dir}/*.h))
BASE_SRCS +=	$(foreach dir,${BASE_DIRS},$(wildcard ${dir}/*.cxx))
BASE_SRCS +=	${GLUE_DIR}/${LIB_SET_NAME}_LibSet.cxx
BASE_OBJS +=	$(addsuffix .o,$(basename ${BASE_SRCS}))

# ROOT Dictionary stuff
DICT_SRCS +=	$(foreach dir,${DICT_DIRS},$(wildcard ${dir}/*.cxx))
DICT_HDRS +=	$(DICT_SRCS:.cxx=.h)
# And these are all secondary files ... created in dict/
DICT_AUTO_BASE:=$(addprefix ${DICT_DIR}/,$(notdir $(basename ${DICT_SRCS})))
DICT_AUTO_SRCS= $(addsuffix .cc,${DICT_AUTO_BASE})
DICT_AUTO_HDRS= $(addsuffix .h, ${DICT_AUTO_BASE})
DICT_AUTO_OBJS=	$(addsuffix .o, ${DICT_AUTO_BASE})

# Views
VIEW_HDRS +=	$(foreach dir,${VIEW_DIRS},$(wildcard ${dir}/*.h))
VIEW_SRCS +=	$(foreach dir,${VIEW_DIRS},$(wildcard ${dir}/*.cxx))
VIEW_SRCS +=	${GLUE_DIR}/${LIB_SET_NAME}_View_LibSet.cxx
VIEW_OBJS +=	$(addsuffix .o,$(basename ${VIEW_SRCS}))
# These are for fluid files (VIEW_FL_FLS) ... fl -> C,H -> o -> View Library
VIEW_FL_FLS  +=	$(foreach dir,${VIEW_FL_DIRS},$(wildcard ${dir}/*.fl))
VIEW_FL_HDRS +=	$(VIEW_FL_FLS:.fl=.H)
VIEW_FL_SRCS +=	$(VIEW_FL_FLS:.fl=.C)
VIEW_FL_SECS =	${VIEW_FL_HDRS} ${VIEW_FL_SRCS}
VIEW_FL_OBJS =	$(VIEW_FL_FLS:.fl=.o)

##########################################################################
# Libset elements
##########################################################################

BASE_LIB_NAME	= lib/lib${LIB_SET_NAME}.so
VIEW_LIB_NAME	= lib/lib${LIB_SET_NAME}_View.so

BASE_LIB_OBJS	= ${BLESSED_OBJS} ${BASE_OBJS} ${DICT_AUTO_OBJS}
VIEW_LIB_OBJS	= ${VIEW_OBJS} ${VIEW_FL_OBJS} ${VIEW_AUTO_OBJS}

${BASE_LIB_NAME}: ${BASE_LIB_OBJS}
	${LD} -o ${BASE_LIB_NAME} -shared ${BASE_LIB_OBJS} \
	  -L${GLEDSYS}/lib ${ABOVE_BASE_LIBS} \
	  ${USER_LD_FLAGS} ${BASE_LD_LIBS}

${VIEW_LIB_NAME}: ${VIEW_LIB_OBJS}
	${LD} -o ${VIEW_LIB_NAME} -shared ${VIEW_LIB_OBJS} \
	  -L${GLEDSYS}/lib ${ABOVE_VIEW_LIBS} \
	  ${USER_LD_FLAGS} ${VIEW_LD_LIBS}

##########################################################################
# Targets
##########################################################################

ALL_SRCS = ${BLESSED_SRCS} ${BASE_SRCS} ${DICT_AUTO_SRCS}
ALL_HDRS = ${BASE_HDRS}
ALL_OBJS = ${BASE_LIB_OBJS}
ALL_LIBS = base_lib

ifndef SKIP_VIEW_LIBS
ALL_SRCS += ${VIEW_SRCS} ${VIEW_FL_SRCS} ${VIEW_AUTO_SRCS}
ALL_HDRS += ${VIEW_HDRS}
ALL_OBJS += ${VIEW_LIB_OBJS}
ALL_LIBS += view_lib
VIEW_SECS = ${VIEW_AUTO_HDRS} ${VIEW_FL_SECS} 
endif

ifndef SKIP_RNR_LIBS
ALL_SRCS += ${RNR_SRCS}
ALL_HDRS += ${RNR_HDRS}
ALL_OBJS += ${RNR_OBJS}
ALL_LIBS += rnr_libs
endif

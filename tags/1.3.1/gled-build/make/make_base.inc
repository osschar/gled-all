# $Id$

##########################################################################
#
# Base make-include file for Gled ... and any add-on libsets
#
##########################################################################

all:	default

include ${GLEDSYS}/make/make_config.inc
include ${GLEDSYS}/make/make_defs.inc
include ${GLEDSYS}/make/make_rnr.inc

-include make_bless.inc
-include make_p7.inc
-include make_dict.inc
-include make_view.inc

LIBSET_DIR	= ${GLEDSYS}/${LIB_SET_NAME}
ABOVE_BASE_LIBS	= $(addprefix -l, ${REQUIRES_LIB_SETS})
ABOVE_VIEW_LIBS	= $(addsuffix _View, $(addprefix -l, ${REQUIRES_LIB_SETS}))


##########################################################################
# Directories to be installed in different contexts
##########################################################################

# Install directories for pre/post-build - symlinked into GLEDSYS
PRE_BUILD_INSTALL_DIRS  += cfg libexec perllib
POST_BUILD_INSTALL_DIRS += bin cfg lib macros perllib
BUILD_INSTALL_DIRS       = $(sort ${PRE_BUILD_INSTALL_DIRS} ${POST_BUILD_INSTALL_DIRS})

# Directories to be installed for blob-install
BLOB_INSTALL_DIRS += lib bin macros cfg perllib

# Directories for proper install
# bin/ and lib/ are handled separately.
INSTALL_DATA_DIRS += cfg macros perllib

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

BASE_LIB_NAME	= lib/lib${LIB_SET_NAME}.${LDEXT}
VIEW_LIB_NAME	= lib/lib${LIB_SET_NAME}_View.${LDEXT}

BASE_LIB_OBJS	= ${BLESSED_OBJS} ${BASE_OBJS} ${DICT_AUTO_OBJS}
VIEW_LIB_OBJS	= ${VIEW_OBJS} ${VIEW_FL_OBJS} ${VIEW_AUTO_OBJS}

${BASE_LIB_NAME}: ${BASE_LIB_OBJS}
	${ECHO} -X- Linking $@
	${MUTE} ${LD} -o $@ ${LDSHAREDOPT} -L${GLEDSYS}/lib ${LDFLAGS} ${BASE_LIB_OBJS} \
     ${ABOVE_BASE_LIBS} ${USER_LD_FLAGS} ${BASE_LD_LIBS}

${VIEW_LIB_NAME}: ${VIEW_LIB_OBJS}
	${ECHO} -X- Linking $@
	${MUTE} ${LD} -o $@ ${LDSHAREDOPT} -L${GLEDSYS}/lib ${LDFLAGS} ${VIEW_LIB_OBJS} \
     ${ABOVE_VIEW_LIBS} ${VIEW_LD_LIBS}

##########################################################################
# Targets
##########################################################################

ifdef USE_PCH
ifeq ($(wildcard ${PCH_SRC_NAME}),${PCH_SRC_NAME})
ALL_SRCS += ${PCH_SRC_NAME}
ALL_OBJS += ${PCH_SRC_NAME}.gch
ALL_DEPS += ${PCH_SRC_NAME}.d
DEPEND_TGTS += ${PCH_SRC_NAME}.gch
endif
endif

ALL_SRCS += ${BLESSED_SRCS} ${BASE_SRCS} ${DICT_AUTO_SRCS}
ALL_HDRS += ${BASE_HDRS}
ALL_OBJS += ${BASE_LIB_OBJS}
ALL_DEPS += $(BASE_LIB_OBJS:.o=.d)
ALL_LIBS += base_lib

ifndef SKIP_VIEW_LIBS
ALL_SRCS += ${VIEW_SRCS} ${VIEW_FL_SRCS} ${VIEW_AUTO_SRCS}
ALL_HDRS += ${VIEW_HDRS}
ALL_OBJS += ${VIEW_LIB_OBJS}
ALL_DEPS += $(VIEW_LIB_OBJS:.o=.d)
ALL_LIBS += view_lib
VIEW_SECS = ${VIEW_AUTO_HDRS} ${VIEW_FL_SECS} 
endif

ifndef SKIP_RNR_LIBS
ALL_SRCS += ${RNR_SRCS}
ALL_HDRS += ${RNR_HDRS}
ALL_OBJS += ${RNR_OBJS}
ALL_DEPS += $(RNR_OBJS:.o=.d)
ALL_LIBS += rnr_libs
endif

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
ALL_HDRS = ${BASE_HDRS} ${VIEW_HDRS} ${RNR_HDRS}
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


default:	${ALL_LIBS} ${LIBSET_TARGETS}
		${MAKE} install

base_lib:	${BASE_LIB_NAME}
	@echo ${LIB_SET_NAME} > lib/.${LIB_SET_ID}

view_lib:	${VIEW_LIB_NAME}

rnr_libs:	${RNR_LIBS}

# Install/uninstall
.PHONY: build_install build_uninstall install uninstall

build_install:
	${GLED_INSTALL} ${GLEDSYS} ${LIBSET_DIR} ${BUILD_INSTALL_DIRS}

build_uninstall:
	${GLED_UNINSTALL} ${GLEDSYS} ${LIBSET_DIR} ${BUILD_INSTALL_DIRS}

install:
	${GLED_INSTALL} ${GLEDSYS} ${LIBSET_DIR} ${INSTALL_DIRS}
	@echo ${LIB_SET_NAME} > ${GLEDSYS}/lib/.${LIB_SET_ID}

uninstall:
	@rm -f ${GLEDSYS}/lib/.${LIB_SET_ID}
	${GLED_UNINSTALL} ${GLEDSYS} ${LIBSET_DIR} ${INSTALL_DIRS}

# like install ... only into a tar file (also Readme_<libset> is added)
# make sure to set environment variable BIN_DIST_TAR
bin_dist:
	@perl -e "die \"must set BIN_DIST_TAR environment variable for target bin_dist\" if \"${BIN_DIST_TAR}\" eq \"\""
	tar cf ${BIN_DIST_TAR} Readme_${LIB_SET_NAME} ${INSTALL_DIRS}

# Depend and pre-depend group
.PHONY:	auto_dirs catalog glue pre_depend depend

auto_dirs:
	@mkdir -p ${GLUE_DIR} ${DICT_DIR} ${VIEW_DIR}

catalog:
	${GLEDCATGEN} ${LIB_SET_NAME} ${LIB_SET_ID} ${BLESSED_DIRS}

glue:
	${GLEDGLUEGEN} ${REQUIRES_LIB_SETS} :: ${RENDERERS}

pre_depend:: auto_dirs catalog glue
	${MKBLESSDEP}
	${MKP7DEP}   ${ALL_HDRS}  > make_p7.inc
	${MKDICTDEP} ${DICT_HDRS} > make_dict.inc

p7dep: ${BLESSED_H7} ${P7_AUTO_HDRS}

depend:	p7dep ${DICT_AUTO_HDRS} ${VIEW_SECS}
	${MKDEP} ${MKDEPFLAGS} -- ${LOCALINCL} -- ${ALL_SRCS} \
	  -f- > make_depend.inc 2>/dev/null

# Cleaners
.PHONY:	dist clean distclean

dist:
	(cd ..; tar czf ${LIB_SET_NAME}-`date +"%y%m%d-%H%M"`.tgz ${LIB_SET_NAME})

clean::
	@rm -f ${ALL_OBJS}

distclean::
	${MAKE} uninstall
	${MAKE} build_uninstall
	${MAKE} clean
	@rm -rf ${DICT_DIR} ${VIEW_DIR} ${GLUE_DIR}
	@rm -rf ${VIEW_FL_SECS} ${BLESSED_H7} ${BLESSED_C7}
	@rm -rf lib/.${LIB_SET_ID} lib/lib* catalog.auto \
		make_bless.inc make_p7.inc make_view.inc make_dict.inc \
		make_depend.inc htmldoc
#	some .h7, .c7 may still be lurking around ... ditch 'em
	find . -name '*.h7' -or -name '*.c7' | xargs rm -f

##########################################################################
# echo group ... to be used w/ grep, perl, xargs ...
##########################################################################

echo_requires_libsets:
	@echo ${REQUIRES_LIB_SETS}
echo_libset_id:
	@echo ${LIB_SET_ID}

echo_blessh:
	@echo ${BLESSED_HDRS}
echo_blesss:
	@echo ${BLESSED_SRCS}
echo_blessh7:
	@echo ${BLESSED_H7}
echo_blesss7:
	@echo ${BLESSED_C7}

echo_baseh:
	@echo ${BASE_HDRS}
echo_bases:
	@echo ${BASE_SRCS}

echo_dict:
	@echo Dict headers:
	@echo ${DICT_HDRS}
	@echo Dict sources:
	@echo ${DICT_SRCS}

echo_dict_dirs:
	@echo ${DICT_DIRS}

echo_fl_fls:
	@echo ${VIEW_FL_FLS}
echo_fl_secs:
	@echo ${VIEW_FL_SECS}

echo_all_stuff:
	@echo All sources:
	@echo ${ALL_SRCS}
	@echo All headers:	
	@echo ${ALL_HDRS}
	@echo All objects:	
	@echo ${ALL_OBJS}
	@echo All libraries:	
	@echo ${ALL_LIBS}
	@echo View secondary files:	
	@echo ${VIEW_SECS}
	@echo Libset targets:
	@echo ${LIBSET_TARGETS}
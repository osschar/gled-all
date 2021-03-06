# $Id$

##########################################################################
# Variables, commands & basic flags
##########################################################################

BUILD_CONFIG	= ${GLEDSYS}/build_config

ifeq ($(wildcard ${BUILD_CONFIG}),)
CONFIG_NOT_DONE = true
else
CONFIG_DONE     = true
endif

ifeq ($(wildcard .depend_done),)
DEPEND_NOT_DONE = true
else
DEPEND_DONE     = true
endif


# CXX, CC taken from environment; LD set by configure
SHELL		= /bin/bash

libexecdir 	= ${GLEDSYS}/libexec
MKDEP		= ${ROOTSYS}/bin/rmkdepend
MKBLESSDEP	= ${libexecdir}/gled_mk_bless_dep.pl
MKP7DEP		= ${libexecdir}/gled_mk_p7_dep.pl
MKDICTDEP	= ${libexecdir}/gled_mk_dict_dep.pl
MKDICTGEN	= ${libexecdir}/gled_mk_dict_gen.pl
P7		= ${libexecdir}/project7.pl
GLEDCATGEN	= ${libexecdir}/gled_cat_gen.pl
GLEDGLUEGEN	= ${libexecdir}/gled_glue_gen.pl

CPPFLAGSGEN	= ${libexecdir}/gled_conf_dumper.pl cppflags

GLED_INSTALL         = ${libexecdir}/gled_install.pl
GLED_UNINSTALL       = ${libexecdir}/gled_uninstall.pl
GLED_INSTALL_FILES   = ${libexecdir}/gled_install_files.pl
GLED_UNINSTALL_FILES = ${libexecdir}/gled_uninstall_files.pl
GLED_INSTALL_DIRS    = ${libexecdir}/gled_install_dirs.pl
GLED_UNINSTALL_DIRS  = ${libexecdir}/gled_uninstall_dirs.pl

PKG_CONFIG      = PKG_CONFIG_PATH=${PKGCFG_DIRS} pkg-config
ROOT_CONFIG     = ${ROOTSYS}/bin/root-config
FLTK_CONFIG     = fltk-config
FLUID   	= fluid


# Flags

LOCALINCL  = -I${GLEDSYS}/include
ifdef CONFIG_DONE
LOCALINCL +=  $(shell ${CPPFLAGSGEN} ${LIB_SET_NAME})
endif

ROOT_CFLAGS  := $(shell ${ROOT_CONFIG} --auxcflags)
ROOT_CPP_DIR := -I$(shell ${ROOT_CONFIG} --incdir)
ROOT_LD_DIR  := -L$(shell ${ROOT_CONFIG} --libdir)
ROOT_LDFLAGS := $(shell ${ROOT_CONFIG} --ldflags)

DEFINES  := -D_GNU_SOURCE

CC  := $(shell ${ROOT_CONFIG} --cc)
CXX := $(shell ${ROOT_CONFIG} --cxx)
LD  := $(shell ${ROOT_CONFIG} --ld)

CPPFLAGS += ${DEFINES} ${LOCALINCL} ${ROOT_CPP_DIR} \
            ${LIBSET_CPPFLAGS} ${USER_CPP_FLAGS}
CXXFLAGS += ${ROOT_CFLAGS} -Wall -fPIC -pipe ${USER_CXX_FLAGS}
LDFLAGS	 += ${ROOT_LDFLAGS} ${USER_LD_FLAGS}

ifeq (${GLED_SYSTEM},Linux)
  LDFLAGS_LIBSETS := -Wl,--no-as-needed
else
  LDFLAGS_LIBSETS :=
endif

# Dictionaries must be compiled without optimization, debug not needed.
# Drop USER_CXX_FLAGS, they should not matter.
DICTCXXFLAGS := ${ROOT_CFLAGS} -Wall -fPIC -pipe

# PCHFLAGS, LDSHAREDOPT, LDEXT set by configure

# ROOT-mkdepend is a copy of old makedepend shipped with X11.
MKDEPFLAGS   := -Y -w 6666

ifdef GLED_ECHO_CMDS
MUTE:=
else
MUTE:=@
endif
ifdef GLED_QUIET
ECHO:=@echo > /dev/null
else
ECHO:=@echo
endif
export MUTE ECHO

##########################################################################
# Basic external libraries
##########################################################################

ROOTLIBS  = $(shell ${ROOT_CONFIG} --noauxlibs --libs)
ROOTGLIBS = $(shell ${ROOT_CONFIG} --noauxlibs --glibs)

# fltk-config started adding lib-patg and -rpath, eg:
#   -L/opt/gled/lib -Wl,-rpath,/opt/gled/lib -lfltk ...
# so we chomp the first two pieces off.
FLTKLIBS  = $(shell ${FLTK_CONFIG} --ldflags --use-gl | perl -pe 's/[^ ]+ [^ ]+ //;') -lGLEW -lfltk-mt
GUILIBS	  = ${FLTKLIBS}

SYSLIBS	= -lcrypto -lssl -lm -ldl

##########################################################################
# Rules
##########################################################################

.SUFFIXES: .C .c .cc .cxx .o .d .fl .h .H .hgs
# .cxx: std extension, .C fluid, .cc dictonary

%.o: %.cc
	${ECHO} -+- Compiling $<
	${MUTE} ${CXX} -c $< ${CPPFLAGS} ${DICTCXXFLAGS} -o $@

%.o: %.cxx
	${ECHO} -*- Compiling $<
	${MUTE} ${CXX} -c $< ${CPPFLAGS} ${CXXFLAGS} ${PCHFLAGS} -o $@

%.pch.h.gch: %.pch.h
	${ECHO} -@- Precompiling $<
	${MUTE} ${CXX} -x c++-header -c $< ${CPPFLAGS} ${CXXFLAGS} -o $@

HC7_SED_FILTER := s![^ ]+\.[hc]7 *!!g

%.d: %.cxx
	${ECHO} -d- Mkdepend $<
	${MUTE} ${MKDEP} ${MKDEPFLAGS} -- ${LOCALINCL} -- $< -f- 2>/dev/null | \
	  perl -pe 's!$*\.o!$*.d $*.o!; ${HC7_SED_FILTER};' > $@

%.pch.h.d: %.pch.h
	${ECHO} -d- Mkdepend $<
	${MUTE} ${MKDEP} ${MKDEPFLAGS} -- ${LOCALINCL} -- $< -f- 2>/dev/null | \
	  perl -pe 's!$*\.pch\.o!$*.pch.h.d $*.pch.h.gch!; ${HC7_SED_FILTER};' > $@

# Fluid rule with fix ... keep intermidiate files
%.C %.H:: %.fl
	${ECHO} -F- Fluid $<
	${MUTE} ${FLUID} -c -o $(subst .fl,.C,$<) -h $(subst .fl,.H,$<) $<

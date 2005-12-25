# $Header$

##########################################################################
# Variables, commands & basic flags
##########################################################################

BUILD_CONFIG	= ${GLEDSYS}/build_config

# CXX, CC taken from environment; LD set by configure
SHELL		= /bin/sh

libexecdir 	= ${GLEDSYS}/lib-exec
MKDEP		= ${ROOTSYS}/bin/rmkdepend
MKBLESSDEP	= ${libexecdir}/mk_bless_dep.pl
MKP7DEP		= ${libexecdir}/mk_p7_dep.pl
MKDICTDEP	= ${libexecdir}/mk_dict_dep.pl
MKDICTGEN	= ${libexecdir}/mk_dict_gen.pl
P7		= ${libexecdir}/project7.pl
GLEDCATGEN	= ${libexecdir}/gled_cat_gen.pl
GLEDGLUEGEN	= ${libexecdir}/gled_glue_gen.pl

CPPFLAGSGEN	= ${libexecdir}/gled_conf_dumper.pl cppflags

GLED_INSTALL	= ${libexecdir}/gled_install.pl
GLED_UNINSTALL	= ${libexecdir}/gled_uninstall.pl

FLUID   	= fluid

# Flags
LOCALINCL = -I${GLEDSYS}/include $(shell ${CPPFLAGSGEN} ${LIB_SET_NAME})
INCL	  = ${LOCALINCL} -I${ROOTSYS}/include -I${X11INCDIR}
CPPFLAGS += -D_REENTRANT -D_GNU_SOURCE -DGL_GLEXT_PROTOTYPES ${USER_CPP_FLAGS} ${INCL}
CXXFLAGS  = -Wall -fPIC -pipe ${USER_CXX_FLAGS}
LDFLAGS	 += ${USER_LD_FLAGS}

# LDSHAREDOPT, LDEXT set by configure

# Pain ... X4 comes with somewhat broken makedepend ... perhaps fixed ??
MKDEPFLAGS	= -Y -w 666

##########################################################################
# Basic external libraries
##########################################################################

# X11LIB, FLLIB, *_CONF set by configure

GLLIB	  = -lGL -lGLU
ROOTLIBS  = -L${ROOTSYS}/lib -lCore -lThread -lCint \
		-lGraf -lGraf3d -lGpad \
		-lHist -lHistPainter -lMatrix -lPhysics \
                -lPostscript -lRint -lTree
ROOTGLIBS = -lGui -lGX11 ${ROOTGLIBS_CONF}

GUILIBS	= ${X11LIB} ${FLLIB} ${GLLIB}
SYSLIBS	= -lcrypto -lssl -lpthread -lm -ldl

##########################################################################
# Rules
##########################################################################

.SUFFIXES: .C .c .cc .cxx .css .o .fl .h .H .hgs .hss
# .cxx: std extension, .C fluid, .cc dictonary

%.o: %.cc
	${CXX} -c $< ${CPPFLAGS} ${CXXFLAGS} -o $@

%.o: %.cxx
	${CXX} -c $< ${CPPFLAGS} ${CXXFLAGS} -o $@

%.o: %.css %.hss
	${CXX} -c $< ${CPPFLAGS} ${CXXFLAGS} -x c++ -o $@

# Fluid rule with fix ... keep intermidiate files
%.C %.H:: %.fl
	${FLUID} -c -o $(subst .fl,.C,$<) -h $(subst .fl,.H,$<) $<

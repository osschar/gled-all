# $Header$

##########################################################################
# Variables, commands & basic flags
##########################################################################

BUILD_CONFIG	= ${GLEDSYS}/build_config

# CXX, CC taken from environment
LD		= ${CXX}

libexecdir 	= ${GLEDSYS}/lib-exec
MKDEP		= makedepend
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
LOCALINCL = -I${GLEDSYS}/include -I. $(shell ${CPPFLAGSGEN} ${LIB_SET_NAME})
INCL	  = ${LOCALINCL} -I${ROOTSYS}/include -I/usr/X11R6/include
CPPFLAGS += -D_REENTRANT -D_GNU_SOURCE ${INCL} ${USER_CPP_FLAGS}
CXXFLAGS  = -Wall -fPIC -pipe ${USER_CXX_FLAGS}
LDFLAGS	 += ${USER_LD_FLAGS}

# Pain ... X4 comes with somewhat broken makedepend ... perhaps fixed ??
MKDEPFLAGS	=-cc /lib/cpp -Y -w 666

##########################################################################
# Basic external libraries
##########################################################################

GLLIB	  = -lGL -lGLU
FLLIB	  = -lfltk -lfltk_gl -lfltk-mt

ROOTLIBS  = -L${ROOTSYS}/lib -lCore -lCint -lGraf -lGraf3d -lGpad \
		-lHist -lHistPainter -lMatrix \
                -lPostscript -lProof -lRint -lTree # -lNew
ROOTGLIBS = -lGui -lGX11

GUILIBS	= -L/usr/X11R6/lib -lX11 -lXext -lXpm ${FLLIB} ${GLLIB}
SYSLIBS	= -lpthread -lm -ldl -rdynamic

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

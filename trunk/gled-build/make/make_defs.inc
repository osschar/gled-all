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
DEFINES   = -D_REENTRANT -D_GNU_SOURCE -DGL_GLEXT_PROTOTYPES
CPPFLAGS += ${DEFINES} ${INCL} ${LIBSET_CPPFLAGS} ${USER_CPP_FLAGS}
CXXFLAGS += -Wall -fPIC -pipe ${USER_CXX_FLAGS}
LDFLAGS	 += ${USER_LD_FLAGS}

# PCHFLAGS, LDSHAREDOPT, LDEXT set by configure

# ROOT-mkdepend is a copy of old makedepend shipped with X11.
MKDEPFLAGS	= -Y -w 6666

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

.SUFFIXES: .C .c .cc .cxx .o .d .fl .h .H .hgs
# .cxx: std extension, .C fluid, .cc dictonary

%.o: %.cc
	${ECHO} -+- Compiling $<
	${MUTE} ${CXX} -c $< ${CPPFLAGS} ${CXXFLAGS} ${PCHFLAGS} -o $@

%.o: %.cxx
	${ECHO} -*- Compiling $<
	${MUTE} ${CXX} -c $< ${CPPFLAGS} ${CXXFLAGS} ${PCHFLAGS} -o $@

%.pch.h.gch: %.pch.h
	${ECHO} -@- Precompiling $<
	${MUTE} ${CXX} -x c++ -c $< ${CPPFLAGS} ${CXXFLAGS} -o $@

%.d: %.cxx
	${ECHO} -d- Mkdepend $<
	${MUTE} ${MKDEP} ${MKDEPFLAGS} -- ${LOCALINCL} -- $< -f- 2>/dev/null | \
	  perl -pe "s!$*\.o!$*.d $*.o!o; s![^\s]+\.[hc]7 *!!og;" > $@

%.d: %.cc
	${ECHO} -d- Mkdepend $<
	${MUTE} ${MKDEP} ${MKDEPFLAGS} -- ${LOCALINCL} -- $< -f- 2>/dev/null | \
	  perl -pe "s!$*\.o!$*.d $*.o!o; s![^\s]+\.[hc]7 *!!og;" > $@

%.pch.h.d: %.pch.h
	${ECHO} -d- Mkdepend $<
	${MUTE} ${MKDEP} ${MKDEPFLAGS} -- ${LOCALINCL} -- $< -f- 2>/dev/null | \
	  perl -pe "s!$*\.pch\.o!$*.pch.h.d $*.pch.h.gch!o; s![^\s]+\.[hc]7 *!!og;" > $@

# Fluid rule with fix ... keep intermidiate files
%.C %.H:: %.fl
	${ECHO} -F- Fluid $<
	${MUTE} ${FLUID} -c -o $(subst .fl,.C,$<) -h $(subst .fl,.H,$<) $<

# $Header$

CXX		= g++
CC		= gcc
LD		= g++
CPPFLAGS	= $(shell make -s -C ${GLEDSYS}/Geom1 echo_cppflags)
CXXFLAGS	= -O -g -Wall -Wno-return-type -fPIC
LDFLAGS		= -shared

SRCS		= $(wildcard *.cxx)
TGTLIBS		= $(SRCS:.cxx=.so)

.SUFFIXES: .cxx .cc .h .o


%.o: %.cc
	${CXX} ${CPPFLAGS} -c -o $@ ${CXXFLAGS} $<
%.o: %.cxx
	${CXX} ${CPPFLAGS} -c -o $@ ${CXXFLAGS} $<

%_Dict.cc %_Dict.h: %.h %_LinkDef.h
	@echo Generating dictionary for $*
	rootcint -f $@ -c $*.h $*_LinkDef.h

%.so: %.o %_Dict.o
	${LD} ${LDFLAGS} -o $@ $^


all: ${TGTLIBS}

clean:
	rm -f *_Dict* *.o *.so

include ${GLEDSYS}/make/make_config.inc
include ${GLEDSYS}/make/make_defs.inc

PCRE_SRCS := $(addprefix pcre/, get.c pcre.c study.c)
PCRE_OBJS := $(PCRE_SRCS:.c=.o)
TPME_SRCS := TPME/TPME.cxx
TPME_OBJS := $(TPME_SRCS:.cxx=.o)
PME_SRCS  := pme/pme.cpp
PME_OBJS  := $(PME_SRCS:.cpp=.o)

OBJS      := ${PCRE_OBJS} ${TPME_OBJS} ${PME_OBJS}

LIB_NAME  := ../lib/libGledCore_pcre.${LDEXT}

all: ${LIB_NAME}

${LIB_NAME}: ${OBJS}
	${ECHO} -X- Linking $@
	${MUTE} ${LD} ${LDSHAREDOPT} -o $@ $^

${PCRE_OBJS}: %.o: %.c
	${ECHO} -*- Compiling $<
	${MUTE} ${CC} -c $< -o $@ -I. -I.. -O2 -fPIC

${TPME_OBJS}: %.o: %.cxx
	${ECHO} -*- Compiling $<
	${MUTE} ${CXX} -c $< -o $@ -I. -I.. -I${ROOTSYS}/include -O2 -fPIC

${PME_OBJS}: %.o: %.cpp
	${ECHO} -*- Compiling $<
	${MUTE} ${CXX} -c $< -o $@ -I. -I.. -O2 -fPIC

clean:
	@rm -f ${OBJS}

distclean:
	@rm -f ${LIB_NAME}

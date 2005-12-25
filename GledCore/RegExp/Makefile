include ${GLEDSYS}/make/make_config.inc
include ${GLEDSYS}/make/make_defs.inc

PCRE_SRCS := get.c pcre.c study.c
TPME_SRCS := TPME.cxx
PME_SRCS  := pme.cpp

OBJS      := $(addprefix pcre/, $(PCRE_SRCS:.c=.o)) \
             $(addprefix TPME/, $(TPME_SRCS:.cxx=.o)) \
             $(addprefix pme/,  $(PME_SRCS:.cpp=.o))

LIB_NAME  := ../lib/libGledCore_pcre.${LDEXT}

all:
	cd pcre; ${CC}  -I. -I.. -O2 -fPIC -c ${PCRE_SRCS}
	cd TPME; ${CXX} -I. -I.. -I${ROOTSYS}/include -O2 -fPIC -c ${TPME_SRCS}
	cd pme;  ${CXX} -I. -I.. -I${ROOTSYS}/include -O2 -fPIC -c ${PME_SRCS}
	${LD} ${LDSHAREDOPT} -o ${LIB_NAME} ${OBJS}

clean:
	rm -f ${OBJS}

distclean:
	rm -f ${LIB_NAME}

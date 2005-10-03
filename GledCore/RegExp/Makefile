PCRE_SRCS := get.c pcre.c study.c
TPME_SRCS := TPME.cxx
PME_SRCS  := pme.cpp
LIB_NAME  := ../lib/libGledCore_pcre.so

all:
	cd pcre; ${CC}  -I. -I.. -O2 -fPIC -c ${PCRE_SRCS}
	cd TPME; ${CXX} -I. -I.. -I${ROOTSYS}/include -O2 -fPIC -c ${TPME_SRCS}
	cd pme;  ${CXX} -I. -I.. -I${ROOTSYS}/include -O2 -fPIC -c ${PME_SRCS}
	find . -name \*.o | xargs ${CXX} -shared -o ${LIB_NAME}

clean:
	find . -name \*.o | xargs rm -f

distclean:
	rm -f ../lib/libGledCore_pcre.so

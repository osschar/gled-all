PCRE_SRCS := get.c pcre.c study.c
PME_SRCS  := pme.cpp
LIB_NAME  := ../lib/libGledCore_pcre.so

all:
	cd pcre; ${CC}  -I. -I.. -O2 -c ${PCRE_SRCS}
	cd pme;  ${CXX} -I. -I.. -O2 -c ${PME_SRCS}
	find . -name \*.o | xargs ${CXX} -shared -o ${LIB_NAME}

clean:
	find . -name \*.o | xargs rm -f

distclean:
	rm -f ../lib/libGledCore_pcre.so

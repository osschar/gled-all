#
# Makefile to produce "gforge"          - jpb 7/28/95
#   stripped image code, added DevIL    - mt  5/23/03


# CC can be set from higher-level makefile or environment

include ${GLEDSYS}/make/make_config.inc

SRCFLAGS=  -c ${USER_CPP_FLAGS}

LIBS    = -lm ${USER_LD_FLAGS} -lIL -lILU

SOURCES=  gforge.c crater.c fftn.c

COMMON=   gforge.h fftn.h

OBJS=     gforge.o crater.o rand.o fftn.o

GFORGE=   ../bin/gforge

all:    ${GFORGE}

${GFORGE}: ${OBJS} 
	${CC} -o ${GFORGE} ${OBJS} ${LIBS}

gforge.o: gforge.c ${COMMON} 
	${CC} ${SRCFLAGS} gforge.c

crater.o: crater.c ${COMMON}
	${CC} ${SRCFLAGS} crater.c

rand.o: rand.c ${COMMON}
	${CC} ${SRCFLAGS} rand.c

fftn.o: fftn.c ${COMMON}
	${CC} ${SRCFLAGS} fftn.c -DFFT_NODOUBLE

clean:
	rm -f ${OBJS}

distclean: clean
	rm -f ${GFORGE}

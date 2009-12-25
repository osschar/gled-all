#
# Makefile to produce "gforge"          - jpb 7/28/95
#   stripped image code, added DevIL    - mt  5/23/03

include ${GLEDSYS}/make/make_config.inc
include ${GLEDSYS}/make/make_defs.inc

CFLAGS  = -DFFT_NODOUBLE -c -O2 ${ROOT_CFLAGS} ${USER_CPP_FLAGS} 

LIBS    = -lm ${USER_LD_FLAGS} -lIL -lILU

SRCS    =  gforge.c crater.c rand.c fftn.c
OBJS    =  $(SRCS:.c=.o)

COMMON=   gforge.h fftn.h

GFORGE=   ../bin/gforge

all:    ${GFORGE}

${GFORGE}: ${OBJS}
	${ECHO} -X- Linking $@
	${MUTE} ${CC} -o ${GFORGE} ${LDFLAGS} ${OBJS} ${LIBS}

${OBJS}: %.o: %.c ${COMMON}
	${ECHO} -c- Compiling $<
	${MUTE} ${CC} $< -o $@ ${CFLAGS}

clean:
	${MUTE} rm -f ${OBJS}

distclean: clean
	${MUTE} rm -f ${GFORGE}

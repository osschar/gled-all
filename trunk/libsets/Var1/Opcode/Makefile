include ${GLEDSYS}/make/make_config.inc
include ${GLEDSYS}/make/make_defs.inc

SRCS := $(wildcard *.cpp) $(wildcard Ice/*.cpp)
OBJS := $(SRCS:.cpp=.o)

LIB  := ../lib/libOpcode.${LDEXT}

all: ${LIB}

${LIB}: ${OBJS}
	${LD} ${LDSHAREDOPT} ${ROOT_LDFLAGS} -o $@ ${OBJS}

clean:
	@rm -f ${OBJS}

distclean: clean
	@rm -f ${LIB}

# Be careful, -O2 causes wrong behaviour on gcc-4.1.
# For example, RayCollider returns zero primitives intersected, so it must
# be that triangle-ray tests are over optimized. Should investigate further.
ifndef OPCODE_DEBUG
CXXFLAGS := -O -fPIC ${ROOT_CFLAGS}
else
CXXFLAGS := -g -fPIC ${ROOT_CFLAGS}
endif

%.o: %.cpp
	${CXX} -I. ${CXXFLAGS} -c $< -o $@



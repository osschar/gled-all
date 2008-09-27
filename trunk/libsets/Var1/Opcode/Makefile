SRCS := $(wildcard *.cpp) $(wildcard Ice/*.cpp)
OBJS := $(SRCS:.cpp=.o)

LIB  := ../lib/libOpcode.so

all: ${LIB}

${LIB}: ${OBJS}
	g++ -shared -o $@ ${OBJS}

clean:
	@rm -f ${OBJS}

distclean: clean
	@rm -f ${LIB}

# Be careful, -O2 causes wrong behaviour on gcc-4.1.
# For example, RayCollider returns zero primitives intersected, so it must
# be that triangle-ray tests are over optimized. Should investigate further.
ifndef OPCODE_DEBUG
CXXFLAGS := -O -fPIC
else
CXXFLAGS := -g -fPIC
endif

%.o: %.cpp
	${CXX} -I. ${CXXFLAGS} -c $< -o $@



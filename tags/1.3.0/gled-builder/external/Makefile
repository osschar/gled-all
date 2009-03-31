SHELL = /bin/bash

PACKAGES := glew root fltk fltk-mt DevIL gts openal freealut

.PHONY: ${PACKAGES} all all-but-root

# Currently disabled packages:
#
# lcms - there was really no need for it.
# DevIL:    lcms


all: ${PACKAGES}

all-but-root: $(filter-out root, ${PACKAGES})

# Package dependencies

fltk-mt:  fltk

freealut: openal

# Targets

${PACKAGES}:
	${MAKE} -C $@

%:
	for pkg in ${PACKAGES}; do ${MAKE} -C $$pkg $@; done

SHELL = /bin/bash

PACKAGES := root fltk fltk-mt DevIL gts openal freealut ehs
LIBSETS  := GledCore Geom1 GledGTS Audio1

.PHONY: ${PACKAGES} ${LIBSETS} all all-but-root

GledCore: root fltk fltk-mt
Geom1:    DevIL
GledGTS:  gts
Audio1:   openal freealut

# Currently disabled packages:
#
# lcms - there was really no need for it.
# DevIL:    lcms
#
# glew - used from root (or from system)

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

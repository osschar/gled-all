PACKAGES := root fltk fltk-mt lcms DevIL gts openal freealut

.PHONY: ${PACKAGES}

all: ${PACKAGES}

# Package dependencies

fltk-mt:  fltk

DevIL:    lcms

freealut: openal

# Targets

${PACKAGES}:
	${MAKE} -C $@

%:
	for pkg in ${PACKAGES}; do ${MAKE} -C $$pkg $@; done

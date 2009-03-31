include ../MakeBuildGled.inc

PACKAGE  := greed

all: install

### install

install:
	${ECHO} -I- Installing ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} rsync -r --exclude '.svn/' top-blob/ ${PREFIX}
	${MUTE} touch $@

### clean, distclean

.PHONY: clean distclean

clean:
	${ECHO} -X- Cleaning  ${PACKAGE}
	${MUTE} rm -f install

distclean: clean

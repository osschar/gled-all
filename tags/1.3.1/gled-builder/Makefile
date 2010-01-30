SHELL = /bin/bash

all: external gled greed

.PHONY: external external-no-root gled greed clean distclean

external:
	${MAKE} -C external

external-no-root:
	${MAKE} -C external all-but-root

gled:
	${MAKE} -C gled

greed:
	${MAKE} -C greed

clean:
	${MAKE} -C external $@
	${MAKE} -C gled $@
	${MAKE} -C greed $@

distclean::
	${MAKE} -C external $@
	${MAKE} -C gled $@
	${MAKE} -C greed $@

distclean::
	rm -f  GledBuildConf.pm MakeBuildConf.inc
	rm -f  configure config.status config.log
	rm -rf autom4te.cache

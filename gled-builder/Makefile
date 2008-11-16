all: external gled

.PHONY: external external-no-root gled clean distclean

external:
	${MAKE} -C external

external-no-root:
	${MAKE} -C external all-but-root

gled:
	${MAKE} -C gled

clean:
	${MAKE} -C external $@
	${MAKE} -C gled $@

distclean::
	${MAKE} -C external $@
	${MAKE} -C gled $@

distclean::
	rm -f  GledBuildConf.pm MakeBuildConf.inc
	rm -f  configure config.status config.log
	rm -rf autom4te.cache

all:
	@echo No resonable default target ... yet.

external:
	${MAKE} -C external

#gled: external
#	${MAKE} -C gled

distclean::
	${MAKE} -C external $@
#	${MAKE} -C gled $@

distclean::
	rm -f  configure config.status config.log GledBuildExternal.pm MakeBuildExternal.inc
	rm -rf autom4te.cache

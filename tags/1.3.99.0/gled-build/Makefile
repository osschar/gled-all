# Top-level makefile for bulding and installing several libsets.

include Makefile.config

ifdef GLED_ECHO_CMDS
MUTE:=
else
MUTE:=@
endif
ifdef GLED_QUIET
ECHO:=@echo > /dev/null
else
ECHO:=@echo
endif
export MUTE ECHO

SHELL                = /bin/bash

libexecdir           = ${GLEDSYS}/libexec

GLED_CONFIG_WRITER   = ${libexecdir}/gled_config_writer.pl
GLED_INSTALL         = ${libexecdir}/gled_install.pl
GLED_UNINSTALL       = ${libexecdir}/gled_uninstall.pl
GLED_INSTALL_FILES   = ${libexecdir}/gled_install_files.pl
GLED_UNINSTALL_FILES = ${libexecdir}/gled_uninstall_files.pl
GLED_INSTALL_DIRS    = ${libexecdir}/gled_install_dirs.pl
GLED_UNINSTALL_DIRS  = ${libexecdir}/gled_uninstall_dirs.pl

########################################################################
# Blob install
########################################################################

blob_install::
	${ECHO} -I- Blob-install
	${MUTE} ${GLED_INSTALL} ${GLED_BLOB_DIR}/perllib perllib ParseConfig.pm
	${MUTE} ${GLED_INSTALL_FILES} -r ${GLED_BLOB_DIR}/demos demos
blob_install::
blob_install::
	${ECHO} -I- Writing \'gled-config\'
	${MUTE} ${GLED_CONFIG_WRITER} ${GLED_BLOB_DIR}/bin/gled-config BLOB

blob_uninstall::
blob_uninstall::
	${ECHO} -I- Blob-uninstall
	${MUTE} ${GLED_UNINSTALL} ${GLED_BLOB_DIR}/perllib ParseConfig.pm
	${MUTE} ${GLED_UNINSTALL_FILES} -r ${GLED_BLOB_DIR}/demos demos
	${MUTE} ${GLED_UNINSTALL} ${GLED_BLOB_DIR}/bin gled-config

########################################################################
# Install
########################################################################

ifneq (${GLED_PREFIX},<none>)

install::	
	${ECHO} -I- Install
	${MUTE} ${GLED_CONFIG_WRITER} ${GLED_BIN_DIR}/bin/gled-config PREFIX \
	        ${GLED_PREFIX} ${GLED_BIN_DIR} ${GLED_LIB_DIR} ${GLED_DATA_DIR}
	${MUTE} ${GLED_INSTALL} ${GLED_DATA_DIR}/perllib perllib ParseConfig.pm
	${MUTE} ${GLED_INSTALL_FILES} -r ${GLED_DATA_DIR}/demos demos
install::

uninstall::
uninstall::
	${ECHO} -I- Uninstall
	${MUTE} ${GLED_UNINSTALL} ${GLED_BIN_DIR} gled-config
	${MUTE} ${GLED_UNINSTALL} ${GLED_DATA_DIR}/perllib ParseConfig.pm
	${MUTE} ${GLED_UNINSTALL_FILES} -r ${GLED_DATA_DIR}/demos demos

endif


########################################################################
# Cleaners
########################################################################

clean::
	rm -f $(addsuffix /.depend_done, ${SUBDIRS})
clean::

distclean::
	rm -f $(addsuffix /.depend_done, ${SUBDIRS})
distclean::
distclean::
	rm -f ${BUILD_FILES}
	rm -rf htmldoc/ classdoc/ macros/*.so


########################################################################
# Helpers
########################################################################

echo_subdirs:
	@echo ${SUBDIRS}

svn_update:
	@svn update
	@svn update libsets
	@svn update demos

echo_svn_modified:
	@(svn status -q; svn status -q libsets demos demos-big) | perl -pe 's/.+[ \t]+//g;'

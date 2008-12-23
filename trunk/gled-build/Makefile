# Top-level makefile for bulding and installing several libsets.

include Makefile.config

########################################################################
# Blob install
########################################################################

blob_install::
blob_install::
	${ECHO} -I- Blob-install NOT IMPLEMENTED FOR TOP LEVEL YET
	${MUTE} ${GLED_INSTALL_DIRS} -r ${GLED_BLOB_DIR} ${LIBSET_DIR} ${BLOB_INSTALL_DIRS}

blob_uninstall::
blob_uninstall::
	${ECHO} -I- Blob-uninstall NOT IMPLEMENTED FOR TOP LEVEL YET

########################################################################
# Install
########################################################################

ifneq (${GLED_PREFIX},<none>)

install::
install::	
	${ECHO} -I- Install NOT IMPLEMENTED FOR TOP LEVEL YET

uninstall::
uninstall::
	${ECHO} -I- Uninstall NOT IMPLEMENTED FOR TOP LEVEL YET
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

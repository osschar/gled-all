# $Id$

##########################################################################
#
# Make-include file for Gled specifying common targets.
#
##########################################################################


all_targets:: ${LIBSET_PREREQS}

all_targets:: ${ALL_LIBS}

all_targets:: ${LIBSET_TARGETS}

default: depend
	${MAKE} all_targets
	${MAKE} post_build_install

base_lib: ${BASE_LIB_NAME}
	@echo ${LIB_SET_NAME} > lib/gled_lid_${LIB_SET_ID}

view_lib: ${VIEW_LIB_NAME}

rnr_libs: ${RNR_LIBS}

# Build-install
.PHONY: pre_build_install post_build_install build_install build_uninstall

pre_build_install::
	${ECHO} -I- Pre-build-install
	${MUTE} ${GLED_INSTALL_DIRS} -rs ${GLEDSYS} ${LIBSET_DIR} ${PRE_BUILD_INSTALL_DIRS}

post_build_install::
	${ECHO} -I- Post-build-install
ifeq (${GLED_SYSTEM},Darwin)
	${MUTE} cd lib; ${GLEDSYS}/libexec/gled_liblink.sh .dylib .so
endif
	${MUTE} ${GLED_INSTALL_DIRS} -rs ${GLEDSYS} ${LIBSET_DIR} ${POST_BUILD_INSTALL_DIRS}

build_install::
	${ECHO} -I- Build-install
	${MUTE} ${GLED_INSTALL_DIRS} -rs ${GLEDSYS} ${LIBSET_DIR} ${BUILD_INSTALL_DIRS}

build_uninstall::
	${ECHO} -I- Build-uninstall
	${MUTE} ${GLED_UNINSTALL_DIRS} -rs ${GLEDSYS} ${LIBSET_DIR} ${BUILD_INSTALL_DIRS}

# Blob install
.PHONY: blob_install blob_uninstall

blob_install::
	${ECHO} -I- Blob-install
	${MUTE} ${GLED_INSTALL_DIRS} -r ${GLED_BLOB_DIR} ${LIBSET_DIR} ${BLOB_INSTALL_DIRS}

blob_uninstall::
	${ECHO} -I- Blob-uninstall
	${MUTE} ${GLED_UNINSTALL_DIRS} -r ${GLED_BLOB_DIR} ${LIBSET_DIR} ${BLOB_INSTALL_DIRS}

# Install
ifneq (${GLED_PREFIX},<none>)
.PHONY: install uninstall

install::	
	${ECHO} -I- Install
	${MUTE} ${GLED_INSTALL_FILES} -r ${GLED_BIN_DIR}  ${LIBSET_DIR}/bin
	${MUTE} ${GLED_INSTALL_FILES} -r ${GLED_LIB_DIR}  ${LIBSET_DIR}/lib
	${MUTE} ${GLED_INSTALL_DIRS}  -r ${GLED_DATA_DIR} ${LIBSET_DIR} ${INSTALL_DATA_DIRS}

uninstall::
	${ECHO} -I- Uninstall
	${MUTE} ${GLED_UNINSTALL_FILES} -r ${GLED_BIN_DIR}  ${LIBSET_DIR}/bin
	${MUTE} ${GLED_UNINSTALL_FILES} -r ${GLED_LIB_DIR}  ${LIBSET_DIR}/lib
	${MUTE} ${GLED_UNINSTALL_DIRS}  -r ${GLED_DATA_DIR} ${LIBSET_DIR} ${INSTALL_DATA_DIRS}
endif

# like install ... only into a tar file (also Readme_<libset> is added)
# make sure to set environment variable BIN_DIST_TAR
bin_dist:
	@perl -e "die \"must set BIN_DIST_TAR environment variable for target bin_dist\" if \"${BIN_DIST_TAR}\" eq \"\""
	if [ ! -e Readme_${LIB_SET_NAME} ]; then touch Readme_${LIB_SET_NAME}; fi
	tar cf ${BIN_DIST_TAR} Readme_${LIB_SET_NAME} ${INSTALL_DIRS}

# Depend and pre-depend group
.PHONY:	auto_dirs catalog glue pre_depend libset_depend depend

auto_dirs:
	${MUTE} mkdir -p ${GLUE_DIR} ${DICT_DIR} ${VIEW_DIR}

catalog: auto_dirs
	${ECHO} -C- Generating glass catalog
	${MUTE} ${GLEDCATGEN} ${LIB_SET_NAME} ${LIB_SET_ID} ${BLESSED_DIRS}

glue: catalog
	${ECHO} -G- Generating glass glue
	${MUTE} ${GLEDGLUEGEN} ${REQUIRES_LIB_SETS} :: ${RENDERERS}

pre_depend:: glue
	${ECHO} -d- Generating p7 dependencies for glasses
	${MUTE} ${MKBLESSDEP}
	${ECHO} -d- Generating p7 dependencies for non-glasses
	${MUTE} ${MKP7DEP}   ${ALL_HDRS}  > make_p7.inc
	${ECHO} -d- Generating ROOT dictionary dependencies
	${MUTE} ${MKDICTDEP} ${DICT_HDRS} > make_dict.inc


p7_tgts:   ${BLESSED_H7} ${P7_AUTO_HDRS} ${VIEW_SECS}

dict_tgts: p7_tgts ${DICT_AUTO_HDRS} 

dep_tgts:  dict_tgts ${ALL_DEPS} ${DEPEND_TGTS}

depend:: dep_tgts
	@touch .depend_done

ifdef DEPEND_DONE
ifeq ($(filter undepend clean distclean, ${MAKECMDGOALS}),)
-include ${ALL_DEPS}
endif
endif

# Cleaners
.PHONY:	dist clean distclean

dist:
	(cd ..; tar czf ${LIB_SET_NAME}-`date +"%y%m%d-%H%M"`.tgz ${LIB_SET_NAME})

undepend:
	${MUTE} find . -name \*.d | xargs rm -f

clean::
	${MUTE} rm -f ${ALL_OBJS} ${ALL_DEPS} .depend_done
	${MUTE} find . -name '*.d' -or -name '*.o' | xargs rm -f

distclean::
	${MAKE} build_uninstall
	${MAKE} clean
	${MUTE} rm -rf ${DICT_DIR} ${VIEW_DIR} ${GLUE_DIR}
	${MUTE} rm -rf ${VIEW_FL_SECS} ${BLESSED_H7} ${BLESSED_C7}
	${MUTE} rm -rf lib/gled_lid_${LIB_SET_ID} lib/lib* catalog.auto \
		make_bless.inc make_p7.inc make_view.inc make_dict.inc htmldoc
#	some .h7, .c7 may still be lurking around ... ditch them
	${MUTE} find . -name '*.h7' -or -name '*.c7' | xargs rm -f

##########################################################################
# echo group ... to be used w/ grep, perl, xargs ...
##########################################################################

echo_requires_libsets:
	@echo ${REQUIRES_LIB_SETS}
echo_libset_id:
	@echo ${LIB_SET_ID}

echo_cppflags:
	@echo ${CPPFLAGS}

echo_bless_hdrs:
	@echo ${BLESSED_HDRS}
echo_bless_srcs:
	@echo ${BLESSED_SRCS}
echo_bless_h7:
	@echo ${BLESSED_H7}
echo_bless_c7:
	@echo ${BLESSED_C7}

echo_baseh:
	@echo ${BASE_HDRS}
echo_bases:
	@echo ${BASE_SRCS}

echo_dict:
	@echo Dict headers:
	@echo ${DICT_HDRS}
	@echo Dict sources:
	@echo ${DICT_SRCS}

echo_dict_dirs:
	@echo ${DICT_DIRS}

echo_fl_fls:
	@echo ${VIEW_FL_FLS}
echo_fl_secs:
	@echo ${VIEW_FL_SECS}

echo_all_stuff:
	@echo All sources:
	@echo ${ALL_SRCS}
	@echo All headers:	
	@echo ${ALL_HDRS}
	@echo All objects:	
	@echo ${ALL_OBJS}
	@echo All libraries:	
	@echo ${ALL_LIBS}
	@echo View secondary files:	
	@echo ${VIEW_SECS}
	@echo Libset targets:
	@echo ${LIBSET_TARGETS}

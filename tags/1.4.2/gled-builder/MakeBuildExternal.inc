########################################################################
# Common configuration, variables and rules for external packages
# This is included from two levels below
#########################################################################

SHELL  = /bin/bash

TOPDIR = ../..

include ${TOPDIR}/MakeBuildConf.inc

########################################################################

ifeq ($(filter clean distclean, ${MAKECMDGOALS}),)
  include make-config.inc
else
  ifneq ($(wildcard unpack),)
    TARDIR := $(shell cat unpack)
  endif
endif

########################################################################
# Common rules
########################################################################

all: install

make-config.inc: ${TOPDIR}/GledBuildExternal.pm
make-config.inc: ${TOPDIR}/MakeBuildExternal.inc
make-config.inc: ${PACKAGE}.pl
	${ECHO} -M- Creating platform specific make-config.inc for ${PACKAGE}
	${MUTE} ./${PACKAGE}.pl > $@ || (rm -f $@ && exit 1)

show-pkg:
	@echo "Package ${PACKAGE}, version ${VERSION}"
	@echo "Src tar file: ${SRCTARFILE}"
	@echo "Top tar dir:  ${TARDIR}"
	@echo "Homepage:     ${HOMEPAGE}"

# unpack

unpack:
	${ECHO} -U- Unpacking ${PACKAGE}, tar-file ${SRCTARFILE}
	${MUTE} rm -rf ${TARDIR} $@
	${MUTE} tar xzf ${CACHE_DIR}/${SRCTARFILE}
	${MUTE} echo ${TARDIR} > $@

# configure

configure: unpack

configure:
	${ECHO} -C- Configuring ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} cd ${TARDIR}; $(configure-cmds)
	${MUTE} touch $@

# build

build: configure

build:
	${ECHO} -B- Building ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} cd ${TARDIR}; $(build-cmds)
	${MUTE} touch $@

# install

install: build

install:
	${ECHO} -I- Installing ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} cd ${TARDIR}; $(install-cmds)
	${MUTE} touch $@

# clean, distclean

.PHONY: clean distclean

clean:
	${ECHO} -X- Cleaning  ${PACKAGE}
ifdef TARDIR
	${MUTE} rm -fr ${TARDIR}
endif
	${MUTE} rm -f  unpack configure build install

distclean: clean
	${ECHO} -X- Distcleaning ${PACKAGE}
	${MUTE} rm -f make-config.inc

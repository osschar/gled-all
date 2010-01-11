include ../MakeBuildGled.inc

PACKAGE  := gled
gled_dir := gled-build

all: install


EXTERNAL := $(shell pwd)/gled-build/external
ifneq (${EXTRA_PATHS},)
EXTERNAL := $(join ${EXTERNAL},:${EXTRA_PATHS})
endif


### checkout

checkout:
	${ECHO} -G- Checking out ${PACKAGE}
	${MUTE} rm -rf ${gled_dir} gled-build-checkout $@
	${MUTE} svn export ${SVN_BASE}/${SVN_DIR}/gled-build-checkout
	${MUTE} ./gled-build-checkout --svn-base=${SVN_BASE} --svn-user=${SVN_USER} ${SVN_DIR}
	${MUTE} touch $@

### configure

define configure-cmds
ln -s ${PREFIX} external;\
. binenv.sh;\
./configure --buildversion="${RELEASE}" --blobdir=${PREFIX}/gled --external=${EXTERNAL} --libsets='<auto>'
endef

configure: checkout

configure:
	${ECHO} -C- Configuring ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} cd ${gled_dir}; $(configure-cmds)
	${MUTE} touch $@

### build

define build-cmds
. binenv.sh;\
${MAKE} ${MAKE_J_OPT}
endef

build: configure

build:
	${ECHO} -B- Building ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} cd ${gled_dir}; $(build-cmds)
	${MUTE} touch $@

### install

define install-cmds
. binenv.sh;\
${MAKE} blob_install
endef

install: build

install:
	${ECHO} -I- Installing ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} cd ${gled_dir}; $(install-cmds)
	${MUTE} touch $@

### clean, distclean

.PHONY: clean distclean

define clean-cmds
. binenv.sh;\
${MAKE} clean
endef


clean:
	${ECHO} -X- Cleaning  ${PACKAGE}
	${MUTE} if test -d ${gled_dir} -a -e configure; then cd ${gled_dir}; $(clean-cmds); fi
	${MUTE} rm -f configure build install

distclean: clean
	${MUTE} rm -f checkout gled-build-checkout
	${MUTE} rm -fr ${gled_dir}

eee:
	echo ${EXTERNAL}
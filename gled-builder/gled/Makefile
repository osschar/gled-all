include ../MakeBuildGled.inc

PACKAGE  := gled
gled_dir := gled-build

all: install


CONFIG_FLAGS :=

ifneq (${EXTRA_PATHS},)
CONFIG_FLAGS += --extrapaths=${EXTRA_PATHS}
endif

ifneq (${ROOT_GLED_DEBUG},)
CONFIG_FLAGS += --debug
endif


### checkout

checkout:
	${ECHO} -G- Checking out ${PACKAGE}
	${MUTE} rm -rf ${gled_dir} gled-build-checkout $@
	${MUTE} svn export ${SVN_BASE}/${SVN_DIR}/gled-build-checkout
	${MUTE} ./gled-build-checkout --svn-base=${SVN_BASE} --svn-user=${SVN_USER} ${SVN_DIR} ${GB_LIBSETS}
	${MUTE} touch $@

### configure

define configure-cmds
ln -sf ${PREFIX} external;\
. binenv.sh;\
./configure --buildversion="${RELEASE}" --blobdir=${PREFIX}\
   --external=${PREFIX} ${CONFIG_FLAGS} --libsets='<auto>'
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
	${MUTE} rsync -r --exclude '.svn/' top-blob/ ${PREFIX}
	${MUTE} touch $@

### update

define update-cmds
. binenv.sh;\
${MAKE} svn_update
endef

update: configure
	${ECHO} -U- Updating ${PACKAGE}
	${MUTE} cd ${gled_dir}; $(update-cmds)
	${MUTE} rm -f build install
	${ECHO} -U- You should run make to build and install the new version.

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

echo_external:
	echo ${EXTERNAL}

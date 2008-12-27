include ../MakeBuildGled.inc

PACKAGE  := gled
gled_dir := gled-build

all: build

### checkout

checkout:
	${ECHO} -G- Checking out ${PACKAGE}
	${MUTE} rm -rf ${gled_dir} gled-build-checkout $@
	${MUTE} svn export http://svn.gled.org/gled/trunk/gled-build-checkout
	${MUTE} ./gled-build-checkout trunk
	${MUTE} touch $@

### configure

define configure-cmds
ln -sf ${PREFIX} external;\
source binenv.sh;\
./configure --external=`pwd`/external --libsets='<auto>'
endef

configure: checkout

configure:
	${ECHO} -C- Configuring ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} cd ${gled_dir}; $(configure-cmds)
	${MUTE} touch $@

### build

define build-cmds
source binenv.sh;\
${MAKE}
endef

build: configure

build:
	${ECHO} -B- Building ${PACKAGE}
	${MUTE} rm -f $@
	${MUTE} cd ${gled_dir}; $(build-cmds)
	${MUTE} touch $@

### clean, distclean

.PHONY: clean distclean

define clean-cmds
source binenv.sh;\
${MAKE} clean
endef


clean:
	${ECHO} -X- Cleaning  ${PACKAGE}
	${MUTE} if test -d ${gled_dir} -a -e configure; then cd ${gled_dir}; $(clean-cmds); fi
	${MUTE} rm -f configure build

distclean: clean
	${MUTE} rm -f checkout gled-build-checkout
	${MUTE} rm -fr ${gled_dir}

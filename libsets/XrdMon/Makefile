# $Header$
LIB_SET_NAME	:= XrdMon
LIB_SET_ID	:= 67
REQUIRES_LIB_SETS := GledCore Net1

# Directories containing glasses - full Gled enabled classes.
BLESSED_DIRS	:= Glasses
# Directories to be included in base library.
BASE_DIRS	:= Stones XrdMon 

# Directories for which dictionaries will be generates.
DICT_DIRS	:= Glasses Stones XrdMon

# Directories to go into the View / GUI library.
VIEW_DIRS	:=
# files from /*.fl are processed by fluid
VIEW_FL_DIRS	:=

## Idiot AMQ installs itself with full version prefix for install dir
## LIBSET_CPPFLAGS = $(shell pkg-config --cflags activemq-cpp)
## Need var with extra pkgconfig paths, external + extra, must fix
## configure and add the var in make/make.config, or sth. Argh etc.
## PKG_CONFIG_PATH=/opt/gled/lib/pkgconfig pkg-config --cflags activemq-cpp
## BASE_LD_LIBS = -lactivemq-cpp -luuid

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_base.inc
#--------------------------------------------------------------

# LIBSET_PREREQS +=
# LIBSET_TARGETS +=

# user targets go here; enumerate them in LIBSET_PREREQS/TARGETS

#--------------------------------------------------------------
include ${GLEDSYS}/make/make_targets.inc
#--------------------------------------------------------------

clean::
distclean::

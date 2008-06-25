CVSROOT=":pserver:anonymous@cvs.gled.org:/u/cvs/gled-1.2"

default:
	echo OoogaDooga nema gleda

all: cvs-co configure build install

cvs-co: gled-build

gled-build:
	cvs -d ${CVSROOT} login
	cvs -d ${CVSROOT} co gled-build
	cd gled-build; cvs -d ${CVSROOT} co GledCore Geom1 Numerica Audio1 GTS Var1

# Something like this for configure:
define configure-cmds
cd gled-build
ln -s $PREFIX external
ln -s external/root .
./configure --external=$PREFIX
endef

# Would be nice to have:
#  - included top level make
#  - available calls to make-maker in GledBuildExternal
#
# Why the hell do we need to run rootcint during configure?
# Some dependency issues? Yes, but I could bypass them somehow.

configure:

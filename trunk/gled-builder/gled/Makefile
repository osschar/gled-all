default:
	echo OoogaDooga nema gleda

all: svn-co configure build install

svn-co:
	cvs co gled-build
	cd gled-build; cvs co GledCore Geom1 Numerica Audio1 GTS Var1

configure:


%:
	${MAKE} -C $@

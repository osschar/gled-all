all: sxrd-classes

.PHONY: sxrd-classes
sxrd-classes:
	./extract-sxrd-classes.pl
	rootcint -f SXrdClasses_Dict.cxx -c -p SXrdClasses.h SXrdClasses_LinkDef.h
	g++ -o libSXrdClasses.so -shared -fPIC `root-config --cflags` SXrdClasses.cxx SXrdClasses_Dict.cxx

clean:
	rm -f libSXrdClasses.so SXrdClasses_Dict.* SXrdClasses.h SXrdClasses.cxx

# To be used with: dir=`pwd`; (cd XXX/demos/XrdMon; cp `make echo-sxrd-classes` $dir)
echo-sxrd-classes:
	@echo SXrdClasses_LinkDef.h SXrdClasses.h SXrdClasses.cxx

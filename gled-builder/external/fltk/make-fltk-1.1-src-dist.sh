#!/bin/sh
#
# make-fltk-src-dist.sh - make a source distribution of fltk
#
# Modified version of fltk-1.1/makesrcdist
# Fix the following variables before running:

url="http://svn.easysw.com/public/fltk/fltk/branches/branch-1.1"
version="1.1.11"
fileurl="ftp://gled.org/cache/HEAD/fltk-$version-gled.tar.gz"

echo Getting sources
rm -rf fltk-$version
svn export $url fltk-$version

cd fltk-$version

fileurl=`echo $fileurl | sed -e '1,$s/\\//\\\\\\//g'`
sed -e '1,$s/%define version.*/%define version '$version'/' \
    -e '1,$s/^Source:.*/Source: '$fileurl'/' \
	<fltk.spec.in >fltk.spec

echo Creating configure script...
autoconf -f

echo Cleaning developer files...
rm -rf OpenGL autom4te* bc5 config forms gl glut images packages themes
rm -f makesrcdist

cd ..

echo "Making UNIX distribution..."

gtar czf fltk-$version-gled.tar.gz fltk-$version

echo "Done!"

#!/bin/sh
#
# Modified version of fltk-1.3/makesrcdist - make a distribution of FLTK.
#

# Fix the following variables before running:

url="http://svn.easysw.com/public/fltk/fltk/branches/branch-1.3"
version="1.3.2-svn"
tarname="fltk-$version-gled.tar.gz"

echo Getting sources
rm -rf fltk-$version
svn export $url fltk-$version

cd fltk-$version

fileurl=`echo $fileurl | sed -e '1,$s/\\//\\\\\\//g'`
sed -e '1,$s/@VERSION@/'$version'/' \
	-e '1,$s/@RELEASE@/'$rev'/' \
	-e '1,$s/^Source:.*/Source: '$fileurl'/' \
	<fltk.spec.in >fltk.spec

echo Creating configure script...
autoconf -f

echo Cleaning developer files...
rm -rf OpenGL autom4te* bc5 config forms glut images packages themes
rm -f  makesrcdist

cd ..

echo "Making UNIX distribution..."
gtar czf $tarname fltk-$version

#echo "Making BZ2 distribution..."
#gtar cjf fltk-$fileversion-source.tar.bz2 fltk-$version

#echo "Making Windows distribution..."
#rm -f fltk-$fileversion-source.zip
#zip -r9 fltk-$fileversion-source.zip fltk-$version

echo "Removing distribution directory..."

rm -rf fltk-$version

echo "Done!"

echo
echo "You might want to run this now:"
echo "  scp $tarname gled.org:/u/ftp/cache/HEAD/"
echo "  rm  $tarname"

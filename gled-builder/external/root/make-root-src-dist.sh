#!/bin/sh
#
# make-root-src-dist.sh - make a source distribution of root
#
# Modified version of root/build/unix/makedistsrc.sh
# Fix the following variables before running:

svnurl="https://root.cern.ch/svn/root/branches/v5-28-00-patches"
version="5.28.01"
dir="root"
tarfile="root-$version-gled.tar"

echo "Checking out ..."

rm -rf $dir
mkdir $dir
svn co $svnurl $dir
cd $dir

# echo "Merging gled-related changes ..."
# For 5.26.01, FTGL stuff
# svn merge -c 32740 $svntrunk

echo "Generating etc/svninfo.txt"
build/unix/svninfo.sh

echo "Remove .svn directories ..."
find . -depth -name .svn -exec rm -rf {} \;
cd ..

echo "Making tarball ..."

tar cf $tarfile $dir
gzip -9 $tarfile

#rm -rf $dir

echo "Done!"

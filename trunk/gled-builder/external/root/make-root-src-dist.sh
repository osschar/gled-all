#!/bin/sh
#
# make-root-src-dist.sh - make a source distribution of root
#
# Modified version of root/build/unix/makedistsrc.sh
# Fix the following variables before running:

svnurl="https://root.cern.ch/svn/root/branches/v5-26-00-patches"
svntrunk="https://root.cern.ch/svn/root/trunk"
version="5.26.01"
dir="root"
tarfile="root-$version-gled.tar"

echo "Checking out ..."

rm -rf $dir
mkdir $dir
svn co $svnurl $dir

echo "Merging gled-related changes ..."
# For 5.26.01
svn merge -c 32740 $svntrunk

echo "Generating etc/svninfo.txt"
cd $dir
build/unix/svninfo.sh

echo "Remove .svn directories ..."
find . -depth -name .svn -exec rm -rf {} \;
cd ..

echo "Making tarball ..."

tar cf $tarfile $dir
gzip -9 $tarfile

#rm -rf $dir

echo "Done!"

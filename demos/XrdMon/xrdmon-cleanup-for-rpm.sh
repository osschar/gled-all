#!/bin/bash

# Clean gled-builder install tree for packaging into a binary RPM.
#
# Needs to be run with current directory equal to install directory, e.g.,
# assuming $installdir:
#   cd $installdir
#   gled/demos/XrdMon/xrdmon-cleanup-for-rpm.sh
#
# Note that we need to leave a significant part of ROOT development files to
# allow for dictionary / streamer generation via aclic.


### Make sure we are in the right directory ###

if [[ -x gled/bin/gled-config && -x root/bin/root-config ]]
then
    echo "Cleaning directory '" `pwd` "' for RPM packaging."
else
    dir=`dirname $0`
    topdir=`readlink -e $dir/../../..`
    echo "$0 directoy '`pwd`' is invalid."
    echo "  Expect to be run in top install directory, probably '$topdir'."
    exit 1
fi


### Erase redundant directories and files ###

rm -rf include/
rm -f  lib/*.a
rm -f  lib/*.la

cd root/

rm -rf test/
rm -rf tutorials/

rm -f lib/libTMVA.*
rm -f lib/libEve.*
rm -f lib/libRGL.*
rm -f lib/libGeom*
rm -f lib/libSmatrix.*
rm -f lib/libProof*

rm -rf include/TMVA/
rm -rf include/GL/

cd cint/cint/lib
find . -name \*.o  | xargs rm -f
find . -name G__\* | xargs rm -f

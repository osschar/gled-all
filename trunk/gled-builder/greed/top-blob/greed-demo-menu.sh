#!/bin/sh

# Menu driven greed-demo runner.
#
# Usage - just run it from anywhere.

########################################################################
# Determine where the script was run from.
########################################################################

# work around readlink versions not having -f option
fullpath1=`readlink $0`
if [ $? -ne 0 ]; then
    fullpath1=$0
fi
progdir=`dirname $fullpath1`
if [ $progdir = "." ]; then
    fullpath=`pwd`
elif echo $progdir | grep "^/" > /dev/null 2>& 1 || \
     echo $progdir | grep "^~" > /dev/null 2>& 1; then
    # absolute path
    fullpath=$progdir
else
    # relative path
    fullpath=`pwd`/$progdir
fi
# work around readlink versions not having -f option
fullpath1=`readlink $fullpath`
if [ $? -ne 0 ]; then
    fullpath1=$fullpath
fi
topdir=$fullpath1

########################################################################
# Setup env
########################################################################

if [ -z "$GLEDSYS" ]; then
    source $topdir/gled.env $topdir
fi

########################################################################
# Query for demo type
########################################################################

tempfile=`mktemp`

dialog --clear --no-cancel --ok-label OK \
       --radiolist "Demo to run:" 10 50 2 \
       tringula.C     "Terrain simulation"      on  \
       solar_system.C "Solar system simulation" off \
  2> $tempfile

demo=`cat $tempfile`; rm $tempfile

if [ $demo = "tringula.C" ]; then

    dialog --clear --no-cancel --ok-label OK \
       --radiolist "Terrain type:" 14 50 6 \
       0 "Rectangle"       off \
       1 "Triangle"        off \
       2 "Sphere outside"  on  \
       3 "Sphere inside"   off \
       4 "Torus outside"   off \
       5 "Torus inside"    off 2> $tempfile
    arg=`cat $tempfile`; rm $tempfile

elif [ $demo = "solar_system.C" ]; then

    dialog --clear --no-cancel --ok-label OK \
       --radiolist "Time integration type:" 10 50 2 \
       0 "Chunked storage"  on  \
       1 "Direct step"      off 2> $tempfile
    arg=`cat $tempfile`; rm $tempfile

else
    echo Unknown demo '$demo'.
    clear
    exit 1
fi

clear

########################################################################
# Run
########################################################################

cd $topdir/gled/demos/Var1
gled $demo\($arg\)

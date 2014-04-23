#!/bin/bash

# Run the default greed-demo
#
# Usage: greed-demo.sh [arguments-for-the-demo]
#   The arguments are passed to the demo script tringula.C.
#   Supported modes:
#       0 "Rectangle"
#       1 "Triangle"
#       2 "Sphere outside"
#       3 "Sphere inside"
#       4 "Torus outside"
#       5 "Torus inside"

########################################################################
# Determine where the script was run from.
########################################################################

# work around readlink versions not having -f option
fullpath1=`readlink "$0"`
if [ $? -ne 0 ]; then
    fullpath1="$0"
fi
progdir=`dirname "$fullpath1"`
if [ "$progdir" = "." ]; then
    fullpath=`pwd`
elif echo $progdir | grep "^/" > /dev/null 2>& 1 || \
     echo $progdir | grep "^~" > /dev/null 2>& 1; then
    # absolute path
    fullpath="$progdir"
else
    # relative path
    fullpath=`pwd`/"$progdir"
fi
# work around readlink versions not having -f option
fullpath1=`readlink "$fullpath"`
if [ $? -ne 0 ]; then
    fullpath1="$fullpath"
fi
topdir="$fullpath1"

########################################################################
# Setup env
########################################################################

if [ -z "$GLEDSYS" ]; then
    . "$topdir/gled-env.sh" "$topdir"
fi

########################################################################
# Run
########################################################################

cd "$topdir/demos/Var1"
gled tringula.C\($*\)

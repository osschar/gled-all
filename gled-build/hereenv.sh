# This is environment for fast gled-build set-up.
# Use as: . hereenv.sh
# Should be run from top Gled directory

dir=`pwd`

export EXTD=$dir/external
export ROOTSYS=$dir/root
export GLEDSYS=$dir

# ROOT
PATH=$ROOTSYS/bin:$PATH
LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH

# External
PATH=$dir/external/bin:$PATH
LD_LIBRARY_PATH=$dir/external/lib:$LD_LIBRARY_PATH
MANPATH=$MANPATH:$dir/external/man

# Gled
PATH=$GLEDSYS/bin:$PATH
LD_LIBRARY_PATH=$GLEDSYS/lib:$LD_LIBRARY_PATH


# OSX specific
export MACOSX_DEPLOYMENT_TARGET=10.4
DYLD_LIBRARY_PATH=$ROOTSYS/lib:$DYLD_LIBRARY_PATH
DYLD_LIBRARY_PATH=$dir/external/lib:$DYLD_LIBRARY_PATH
DYLD_LIBRARY_PATH=$GLEDSYS/lib:$DYLD_LIBRARY_PATH

export PATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH MANPATH

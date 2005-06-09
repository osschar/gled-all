# This is environment for fast gled-build set-up.
# Use as: . hereenv.sh
# Should be run from top Gled directory

dir=`pwd`

export EXTD=$dir/external
export ROOTSYS=$dir/root
export GLEDSYS=$dir

# ROOT
PATH=$PATH:$ROOTSYS/bin
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ROOTSYS/lib

# External
PATH=$PATH:$dir/external/bin
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$dir/external/lib
MANPATH=$MANPATH:$dir/external/man

# Gled
PATH=$PATH:$GLEDSYS/bin
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GLEDSYS/lib

export PATH LD_LIBRARY_PATH MANPATH

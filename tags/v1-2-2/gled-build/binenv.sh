# This is environment for binary gled base distribution.
# Use as: . binenv.sh
# Should be run from top Gled directory

dir=`pwd`

export ROOTSYS=$dir/external/root
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

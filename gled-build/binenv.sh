# This is environment for binary gled base distribution.
# Use as: . binenv.sh
# Should be run from top Gled directory

dir=`pwd`

export ROOTSYS=$dir/external/root
export GLEDSYS=$dir

PATH=$GLEDSYS/bin:$ROOTSYS/bin:$dir/external/bin:$PATH

LD_LIBRARY_PATH=$GLEDSYS/lib:$ROOTSYS/lib:$dir/external/lib:$LD_LIBRARY_PATH

MANPATH=$dir/external/man:$MANPATH

export PATH LD_LIBRARY_PATH MANPATH

# Setup environment for sh.
#
# Usage: . gled-env.sh [blob-directory]
#   if blob-directory is not given, current directory is used.

########################################################################
# Determine the top-directory
########################################################################

if [ $# -eq 0 ]; then
  topdir=`pwd`
else
  topdir="$1"
fi

########################################################################
# Setup the variables
########################################################################

export ROOTSYS="$topdir/root"
export GLEDSYS="$topdir/gled"
PATH=$GLEDSYS/bin:$ROOTSYS/bin:$topdir/bin:$PATH
export LD_LIBRARY_PATH=$GLEDSYS/lib:$ROOTSYS/lib:$topdir/lib:$LD_LIBRARY_PATH
export MANPATH=$ROOTSYS/man:$topdir/man:$MANPATH

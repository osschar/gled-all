# This is environment for using exisitng external software and ROOT.
# It expects external software in external/ and ROOT in root/.
# Usage: . hereenv.sh
# Should be run from the 'gled-build' directory.

dir=`pwd`

export EXTD=$dir/external
export ROOTSYS=$dir/root
export GLEDSYS=$dir

# External
PATH=$dir/external/bin:$PATH
LD_LIBRARY_PATH=$dir/external/lib:$LD_LIBRARY_PATH
MANPATH=$MANPATH:$dir/external/man

# ROOT
PATH=$ROOTSYS/bin:$PATH
LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH

# Gled
PATH=$GLEDSYS/bin:$PATH
LD_LIBRARY_PATH=$GLEDSYS/lib:$LD_LIBRARY_PATH

export LD_LIBRARY_PATH MANPATH

# OSX specific, probably obsolete
#export MACOSX_DEPLOYMENT_TARGET=10.4
#DYLD_LIBRARY_PATH=$ROOTSYS/lib:$DYLD_LIBRARY_PATH
#DYLD_LIBRARY_PATH=$dir/external/lib:$DYLD_LIBRARY_PATH
#DYLD_LIBRARY_PATH=$GLEDSYS/lib:$DYLD_LIBRARY_PATH
#export DYLD_LIBRARY_PATH

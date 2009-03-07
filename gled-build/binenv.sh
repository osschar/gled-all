# This is environment for standard gled build.
# It expects external software in external/ and ROOT in external/root/.
# Usage: . binenv.sh
# Should be run from 'gled-build' directory.

dir=`pwd`

export ROOTSYS="$dir/external/root"
export GLEDSYS="$dir"

PATH="$GLEDSYS"/bin:"$ROOTSYS"/bin:"$dir"/external/bin:$PATH

LD_LIBRARY_PATH="$GLEDSYS"/lib:"$ROOTSYS"/lib:"$dir"/external/lib:$LD_LIBRARY_PATH

MANPATH="$dir"/external/man:$MANPATH

export PATH LD_LIBRARY_PATH MANPATH

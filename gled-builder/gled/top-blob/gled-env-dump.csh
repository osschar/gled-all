#!/bin/bash

# Write environment for 'csh' shells to stdout.
# Location of Gled install is automatically detected.
# Usage:
#   a) dump to a file (to either source it or add it to .profile)
#      /path/to/gled-blob/gled-env-dump.csh > somefile
#   b) direct setting of environment vie eval
#      eval `/path/to/gled-blob/gled-env-dump.csh`

########################################################################
# Determine the top-directory
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
# Setup the variables
########################################################################

rsys="$topdir/root"
gsys="$topdir"

cat <<EOF
setenv ROOTSYS "$rsys"
setenv GLEDSYS "$gsys"

setenv PATH "$gsys"/bin:"$rsys"/bin:"$topdir"/bin:\$PATH

if ( \$?LD_LIBRARY_PATH ) then
  setenv LD_LIBRARY_PATH "$gsys"/lib:"$rsys"/lib:"$topdir"/lib:\$LD_LIBRARY_PATH
else
  setenv LD_LIBRARY_PATH "$gsys"/lib:"$rsys"/lib:"$topdir"/lib
endif

if ( \$?DYLD_LIBRARY_PATH ) then
  setenv DYLD_LIBRARY_PATH "$gsys"/lib:"$rsys"/lib:"$topdir"/lib:\$DYLD_LIBRARY_PATH
else
  setenv DYLD_LIBRARY_PATH "$gsys"/lib:"$rsys"/lib:"$topdir"/lib
endif

if ( \$?MANPATH ) then 
  setenv MANPATH "$rsys"/man:"$topdir"/man:\$MANPATH
else
  setenv MANPATH "$rsys"/man:$topdir/man
endif
EOF

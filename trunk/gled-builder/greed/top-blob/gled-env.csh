#!/bin/csh

# Setup environment for csh.
#
# Usage: source gled-env.csh [blob-directory]
#   if blob-directory is not given, current directory is used.

########################################################################
# Determine the top-directory
########################################################################

if ($#argv == 0) then
    set topdir = `pwd`
else
    set topdir = $argv[1]
endif

########################################################################
# Setup the variables
########################################################################

setenv ROOTSYS ${topdir}/root
setenv GLEDSYS ${topdir}/gled

setenv PATH ${GLEDSYS}/bin:${ROOTSYS}/bin:${topdir}/bin:${PATH}

if ( $?LD_LIBRARY_PATH ) then
  setenv LD_LIBRARY_PATH ${GLEDSYS}/lib:${ROOTSYS}/lib:${topdir}/lib:${LD_LIBRARY_PATH}
else
  setenv LD_LIBRARY_PATH ${GLEDSYS}/lib:${ROOTSYS}/lib:${topdir}/lib
endif

if ( $?MANPATH ) then 
  setenv MANPATH ${ROOTSYS}/man:${topdir}/man:${MANPATH}
else
  setenv MANPATH ${ROOTSYS}/man:${topdir}/man
endif

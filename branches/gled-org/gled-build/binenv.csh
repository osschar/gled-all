# This is environment for binary gled base distribution.
# Use as: source binenv.csh
# Should be run from top Gled directory

set dir = `pwd`

setenv ROOTSYS ${dir}/external/root
setenv GLEDSYS ${dir}

# External
setenv PATH ${PATH}:${dir}/external/bin
if ( $?LD_LIBRARY_PATH ) then
  setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${dir}/external/lib
else
  setenv LD_LIBRARY_PATH ${dir}/external/lib
endif
if ( $?MANPATH ) then 
  setenv MANPATH ${MANPATH}:${dir}/external/man
else
  setenv MANPATH ${dir}/external/man
endif

# ROOT
setenv PATH ${PATH}:${ROOTSYS}/bin
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${ROOTSYS}/lib

# Gled
setenv PATH ${PATH}:${GLEDSYS}/bin
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${GLEDSYS}/lib

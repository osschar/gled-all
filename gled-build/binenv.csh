# This is environment for standard gled build.
# It expects external software in external/ and ROOT in external/root/.
# Usage: source binenv.csh
# Should be run from 'gled-build' directory.

set dir = `pwd`

setenv ROOTSYS ${dir}/external/root
setenv GLEDSYS ${dir}

setenv PATH ${GLEDSYS}/bin:${ROOTSYS}/bin:${dir}/external/bin:${PATH}

if ( $?LD_LIBRARY_PATH ) then
  setenv LD_LIBRARY_PATH ${GLEDSYS}/lib:${ROOTSYS}/lib:${dir}/external/lib:${LD_LIBRARY_PATH}
else
  setenv LD_LIBRARY_PATH ${GLEDSYS}/lib:${ROOTSYS}/lib:${dir}/external/lib
endif

if ( $?MANPATH ) then 
  setenv MANPATH ${dir}/external/man:${MANPATH}
else
  setenv MANPATH ${dir}/external/man
endif

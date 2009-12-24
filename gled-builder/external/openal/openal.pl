#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "openal";

$homepage = "http://openal.org/";

$comment  = <<"FNORD";
Cross-platform 3D audio library.
Released under GNU LGPL.
FNORD

$parallel = 1;

setup_package($package);

# Had once also: --enable-vorbis --enable-capture --enable-paranoid-locks

# Eventually also specfy build-type as Release, default is:
# CMAKE_BUILD_TYPE Debug Release RelWithDebInfo MinSizeRel

target('configure', <<"FNORD");
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=\${PREFIX} \\
      -DCMAKE_BUILD_TYPE=Release \\
      -DCMAKE_C_FLAGS=$CFLAGS \\
      -DCMAKE_EXE_LINKER_FLAGS=$LDFLAGS \\
      -DCMAKE_SHARED_LINKER_FLAGS=$LDFLAGS \\
      ..
FNORD

target('build', <<"FNORD");
cd build
\${MAKE}
FNORD

target('install', <<"FNORD");
cd build
\${MAKE} install
FNORD

use_defaults_for_remaining_targets("cd CMakeConf");

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

# OSX ships with OpenAL framework, do nothing.
if ($BUILD_OS =~ /darwin/)
{
  target('configure', "");
  target('build', "");
  target('install', "");
  `touch unpack configure build install`;
  exit 0;
}

# Had once also: --enable-vorbis --enable-capture --enable-paranoid-locks

# CMAKE_BUILD_TYPE: Debug Release RelWithDebInfo MinSizeRel

target('configure', <<"FNORD");
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=\${PREFIX} \\
      -DCMAKE_BUILD_TYPE=Release \\
      -DCMAKE_C_FLAGS=$CFLAGS \\
      -DCMAKE_EXE_LINKER_FLAGS=$LDFLAGS \\
      -DCMAKE_SHARED_LINKER_FLAGS=$LDFLAGS \\
      ..
FNORD

use_defaults_for_remaining_targets("cd build");

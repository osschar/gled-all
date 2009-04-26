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
# CMAKE_BUILD_TYPEFIX              RelWithDebInfo

target('configure', <<"FNORD");
cd CMakeConf
cmake -DCMAKE_INSTALL_PREFIX:PATH=\${PREFIX} ..
FNORD

use_defaults_for_remaining_targets("cd CMakeConf");

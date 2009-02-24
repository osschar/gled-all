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
target('configure', <<"FNORD");
./configure CFLAGS="-fPIC -O2" --prefix=\${PREFIX}
FNORD

use_defaults_for_remaining_targets();

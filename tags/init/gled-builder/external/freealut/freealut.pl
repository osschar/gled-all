#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "freealut";

$homepage = "http://openal.org/";

$comment  = <<"FNORD";
Utility library for OpenAL, a cross-platform 3D audio library.
Released under GNU LGPL.
FNORD


setup_package($package);

target('configure', <<"FNORD");
./configure   CPPFLAGS=-I\${PREFIX}/include \\
              CFLAGS="-O2 -fPIC" \\
              LDFLAGS=-L\${PREFIX}/lib \\
            --prefix=\${PREFIX}
FNORD

use_defaults_for_remaining_targets();

#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "lcms";

$homepage = "http://www.littlecms.com/";

$comment  = <<"FNORD";
Little CMS: a small, speed optimized color management engine.
FNORD


setup_package($package);

target('configure', <<"FNORD");
./configure   CFLAGS="-O2 -fPIC" --prefix=\${PREFIX}
FNORD

target('install', <<"FNORD");
make install
(cd \${PREFIX}/include; ln -s . lcms)
FNORD

use_defaults_for_remaining_targets();

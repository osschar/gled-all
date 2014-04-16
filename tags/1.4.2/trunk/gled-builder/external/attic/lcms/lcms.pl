#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "lcms";

$homepage = "http://www.littlecms.com/";

$comment  = <<"FNORD";
Little CMS: a small, speed optimized color management engine.
FNORD

$parallel = 1;

setup_package($package);

add_cflags("-O2 -fPIC");

target('install', <<"FNORD");
make install
(cd \${PREFIX}/include; rm -rf lcms; ln -s . lcms)
FNORD

use_defaults_for_remaining_targets();

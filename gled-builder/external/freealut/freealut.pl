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

$parallel = 1;

setup_package($package);

add_search_path("\${PREFIX}");

add_cflags("-O2 -fPIC");

use_defaults_for_remaining_targets();

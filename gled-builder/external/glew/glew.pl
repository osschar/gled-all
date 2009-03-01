#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "glew";

$homepage = "http://glew.sourceforge.net/";

$comment  = <<"FNORD";
A cross-platform open-source C/C++ extension loading library.
Released under Modified BSD License, the Mesa 3-D License, and the Khronos License.
FNORD

$parallel = 0;

setup_package($package);

target('configure', "echo Configure not needed.");

target('install', <<"FNORD");
GLEW_DEST=${PREFIX} make install
FNORD

use_defaults_for_remaining_targets();

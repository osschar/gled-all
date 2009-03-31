#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "fltk";

$homepage = "http://www.fltk.org/";

$comment  = <<"FNORD";
Fast Light ToolKit: a C++ widget library.
Released under FLTK license (mostly GNU LGPL v2).
FNORD

$parallel = 1;

setup_package($package);

# print STDERR "Package setup, srctar=$srctarfile, ver=$version, tardir=$tardir\n";

target('configure', <<"FNORD");
./configure   CXXFLAGS="-fPIC -O2" \\
            --prefix=\${PREFIX} \\
            --enable-shared --enable-threads --enable-xft
FNORD

use_defaults_for_remaining_targets();

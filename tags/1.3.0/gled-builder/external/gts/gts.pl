#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "gts";

$homepage = "http://gts.sourceforge.net/";

$comment  = <<"FNORD";
GNU Triangulated Surface Library.
Released under GNU LGPL.
FNORD

$parallel = 1;

setup_package($package);

my $cppflags;
if ($DISTRO_VENDOR eq 'gentoo') {
  $cppflags = "CPPFLAGS=-I/usr/include/netpbm";
}

target('configure', <<"FNORD");
NOCONFIGURE=true bash autogen.sh
./configure $cppflags CXXFLAGS=-O2 --prefix=\${PREFIX}
FNORD

use_defaults_for_remaining_targets();

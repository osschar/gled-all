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


setup_package($package);

my $cppfl;
if ($DISTRO_VENDOR eq 'gentoo') {
  $cppfl .= "-I/usr/include/netpbm";
}
target('configure', <<"FNORD");
./configure   CXXFLAGS=-O2 CPPFLAGS="$cppfl" \\
            --prefix=\${PREFIX}
FNORD

use_defaults_for_remaining_targets();

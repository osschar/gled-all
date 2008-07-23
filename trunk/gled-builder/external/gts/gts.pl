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
  target('configure', <<"FNORD");
bash autogen.sh
./configure CPPFLAGS=-I/usr/include/netpbm  CXXFLAGS=-O2 --prefix=\${PREFIX}
FNORD
} else {
  target('configure', <<"FNORD");
./configure CXXFLAGS=-O2 --prefix=\${PREFIX}
FNORD
}

use_defaults_for_remaining_targets();

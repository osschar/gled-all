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

if ($DISTRO_VENDOR eq 'gentoo')
{
  add_cppflags("-I/usr/include/netpbm");
}

add_cxxflags("-O2");

target_configure("NOCONFIGURE=true bash autogen.sh");

use_defaults_for_remaining_targets();

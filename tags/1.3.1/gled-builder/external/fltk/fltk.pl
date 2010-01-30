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

add_cxxflags("-fPIC -O2");
add_cfgflags("--enable-shared --enable-threads");

if ($BUILD_OS =~ /linux/)
{
  add_cfgflags("--enable-xft");
}
elsif ($BUILD_OS =~ /darwin/)
{
  add_cfgflags("--enable-quartz");
}

# Install of fltk wipes include/FL - so fltk-mt needs to be
# reinstalled after install of fltk.
$xxdir  = "../../fltk-mt";
$xxfile = "$xxdir/install";

target('install', <<"FNORD");
make install
if test -e $xxfile; then rm $xxfile; make -C $xxdir install; fi
FNORD

use_defaults_for_remaining_targets();

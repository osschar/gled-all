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

my $config_args;
if ($BUILD_OS =~ /linux/)
{
  $config_args .= "--enable-xft";
}
elsif ($BUILD_OS =~ /darwin/)
{
  $config_args .= "--enable-quartz";
}

target('configure', <<"FNORD");
./configure   CXXFLAGS="-fPIC -O0 -g" \\
            --prefix=\${PREFIX} \\
            --enable-shared --enable-threads $config_args
FNORD

use_defaults_for_remaining_targets();

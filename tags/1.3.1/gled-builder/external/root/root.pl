#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "root";

$homepage = "http://root.cern.ch/";

$comment  = <<"FNORD";
OO framework for data analysis, storage and more.
Released under LGPL-2.1.
FNORD

$parallel = 1;

setup_package($package);

my $config_args;
if ($BUILD_OS =~ /linux/)
{
  $config_args .= "--disable-builtin-freetype";
  if ($DISTRO_VENDOR eq 'gentoo') {
    $config_args .= " --enable-gsl-shared --with-pgsql-incdir=/usr/include/postgres";
  }
  elsif ($DISTRO_VENDOR eq 'debian') {
    $config_args .= " --enable-gsl-shared";
  }
}
elsif ($BUILD_OS =~ /darwin/)
{
  # Enforce 32-bit build ... other externals build in 32-bit mode.
  # Another option would be to enforce 64-bit mode for them, too.
  # But then would need to detect the cpu type ... or sth.
  $config_args .= "macosx";
}

target('configure', <<"FNORD");
./configure $config_args
FNORD

target('install', <<"FNORD");
ROOTSYS=${PREFIX}/root make install
FNORD

use_defaults_for_remaining_targets();

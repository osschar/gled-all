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

my $config_args = "--disable-builtin-freetype";
if ($DISTRO_VENDOR eq 'gentoo') {
  $config_args .= " --enable-gsl-shared --with-pgsql-incdir=/usr/include/postgres";
}
target('configure', <<"FNORD");
./configure $config_args
FNORD

target('install', <<"FNORD");
ROOTSYS=${PREFIX}/root make install
FNORD

use_defaults_for_remaining_targets();

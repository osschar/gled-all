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
my $pre_install;

$config_args .= " --build=debug" if $ROOT_GLED_DEBUG;
$config_args .= " --disable-builtin-freetype --disable-castor";

if ($BUILD_OS =~ /linux/)
{
  if ($DISTRO_VENDOR eq 'gentoo') {
    $config_args .= " --enable-gsl-shared --with-pgsql-incdir=/usr/include/postgres";
  }
  elsif ($DISTRO_VENDOR eq 'debian') {
    $config_args .= " --enable-gsl-shared";
  }
}
elsif ($BUILD_OS =~ /darwin/)
{
  # Starting with 10.7, ftgl and glew no longer work with fltk when configured
  # to use GLX. So we boldly hack around.
  # The side-effect is that root gl and eve do not work. Sigh, or sth.
  $pre_install = <<"FNORD"
echo "BUILDGL        := yes" >> config/Makefile.config
echo "OPENGLLIBDIR   :=" >> config/Makefile.config
echo "OPENGLULIB     :=" >> config/Makefile.config
echo "OPENGLLIB      := -framework AGL -framework OpenGL" >> config/Makefile.config
echo "OPENGLINCDIR   :=" >> config/Makefile.config
perl -ni -e 'print unless /define GLEW_APPLE_GLX/o;' graf3d/glew/src/glew.c
rm -f lib/libGLEW.* lib/libFTGL.*
make
FNORD
}

target('configure', <<"FNORD");
./configure $config_args
FNORD

target('install', <<"FNORD");
$pre_install
ROOTSYS=${PREFIX}/root make install
FNORD

use_defaults_for_remaining_targets();

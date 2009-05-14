#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "DevIL";

$homepage = "http://www.imagelib.org/";

$comment  = <<"FNORD";
Developers Image Library: supports many image operations.
Released under GNU LGPL v2.1.
FNORD

$parallel = 1;

setup_package($package);

my $cppfl = "-I\${PREFIX}/include";
my $cfl   = "-O2 -fPIC";
my $ldfl  = "-L\${PREFIX}/lib";
if ($CC eq 'gcc' and $CC_MAJOR == 4 and $CC_MINOR >= 2) {
  $cfl .= " -fgnu89-inline";
}

my $config_args;
if ($BUILD_OS =~ /linux/)
{
  if ($DISTRO_VENDOR eq 'redhat' and $DISTRO_NAME eq 'slc') {
    $ldfl .= " -L/usr/X11R6/lib";
  }
  $ldfl .= " -ljpeg -ltiff -lmng";
  $config_args .= "--enable-ILU --enable-ILUT";
}
elsif ($BUILD_OS =~ /darwin/)
{
  # ILUT has trouble linking against GL. Not used in Gled anyway.
  $config_args .= "--enable-ILU";
}

target('configure', <<"FNORD");
./configure CPPFLAGS="$cppfl" \\
            CFLAGS="$cfl" \\
            LDFLAGS="$ldfl" \\
          --prefix=\${PREFIX} $config_args
FNORD

use_defaults_for_remaining_targets();

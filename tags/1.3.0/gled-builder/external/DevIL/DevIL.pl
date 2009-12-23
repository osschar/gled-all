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
if ($BUILD_OS eq 'linux-gnu') {
  if ($DISTRO_VENDOR eq 'redhat' and $DISTRO_NAME eq 'slc') {
    $ldfl .= " -L/usr/X11R6/lib";
  }
  $ldfl .= " -ljpeg -ltiff -lmng";
}
target('configure', <<"FNORD");
./configure CPPFLAGS="$cppfl" \\
            CFLAGS="$cfl" \\
            LDFLAGS="$ldfl" \\
          --prefix=\${PREFIX} --enable-ILU --enable-ILUT
FNORD

use_defaults_for_remaining_targets();
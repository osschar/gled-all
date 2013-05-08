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

add_search_path("\${PREFIX}");

add_cflags  ("-fPIC");

if ($CC eq 'gcc' and $CC_MAJOR == 4 and $CC_MINOR >= 2)
{
  add_cflags("-fgnu89-inline");
}

if ($BUILD_OS =~ /linux/)
{
  if ($DISTRO_VENDOR eq 'redhat' and $DISTRO_NAME eq 'slc') {
    add_ldflags ("-L/usr/X11R6/lib");
  }
  add_ldflags ("-ljpeg -ltiff -lmng");
  add_cfgflags("--enable-ILU --enable-ILUT");
}
elsif ($BUILD_OS =~ /darwin/)
{
  # ILUT has trouble linking against GL. Not used in Gled anyway.
  add_cfgflags("--enable-ILU");
}

target_configure($config_args);

use_defaults_for_remaining_targets();

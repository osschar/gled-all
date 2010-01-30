#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "fltk-mt";

$homepage = "http://www.gled.org/fltk-mt/";

$comment  = <<"FNORD";
A collection of FLTK widgets and utilities.
Free software ... must keep licenses in source.
FNORD

$parallel = 1;

setup_package($package);

my $ext = "\${PREFIX}";
if ($EXTRA_PATHS ne "")
{
  $ext .= ":${EXTRA_PATHS}";
}

target('configure', <<"FNORD");
./configure --prefix=\${PREFIX} --external=\${PREFIX}
FNORD

use_defaults_for_remaining_targets();

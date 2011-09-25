#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "ehs";

$homepage = "http://sourceforge.net/projects/ehs/";

$comment  = <<"FNORD";
Embedded HTTP Server.
FNORD

$parallel = 1;

setup_package($package);

# add_cflags("-O2 -fPIC");

use_defaults_for_remaining_targets();

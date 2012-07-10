#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "apr-util";

$homepage = "http://apr.apache.org/";

$comment  = <<"FNORD";
The mission of the Apache Portable Runtime (APR) project is to create and
maintain software libraries that provide a predictable and consistent
interface to underlying platform-specific implementations.
Language: C
FNORD

$parallel = 1;

setup_package($package);

add_cflags("-O2");

add_cfgflags("--with-apr=\${PREFIX}");

use_defaults_for_remaining_targets();

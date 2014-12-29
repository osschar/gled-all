#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "activemq-cpp";

$homepage = "http://activemq.apache.org/cms/";

$comment  = <<"FNORD";
Apache ActiveMQ CPP is the C++ client for accessing the ActiveMQ message broker.
Language: C++
FNORD

$parallel = 1;

setup_package($package);

add_cxxflags("-O2");

add_cfgflags("--with-apr=\${PREFIX}");
add_cfgflags("--with-apr-util=\${PREFIX}");

use_defaults_for_remaining_targets();

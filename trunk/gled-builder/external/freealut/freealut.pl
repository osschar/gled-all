#!/usr/bin/perl

# Output of this file is dumped into make-config.inc

use lib "../..";
use GledBuildExternal;

$package  = "freealut";

$homepage = "http://openal.org/";

$comment  = <<"FNORD";
Utility library for OpenAL, a cross-platform 3D audio library.
Released under GNU LGPL.
FNORD

$parallel = 1;

setup_package($package);

add_search_path("\${PREFIX}");

add_cflags("-O2 -fPIC");

print STDERR "CPPFLAGS = $CPPFLAGS\n";
print STDERR "CFLAGS   = $CFLAGS\n";
print STDERR "CXXFLAGS = $CXXFLAGS\n";
print STDERR "LDFLAGS  = $LDFLAGS\n";
print STDERR "CFGFLAGS = $CFGFLAGS\n";

### -I/System/Library/Frameworks/OpenAL.framework/Headers" \\
### -DOPENAL_LIB="/System/Library/Frameworks/OpenAL.framework" \\

target('configure', <<"FNORD");
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH="\${PREFIX}" \\
-DCMAKE_BUILD_TYPE="Release" \\
-DCMAKE_C_FLAGS="$CPPFLAGS $CFLAGS" \\
-DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" \\
-DCMAKE_SHARED_LINKER_FLAGS="$LDFLAGS" \\
-DCMAKE_VERBOSE_MAKEFILE="true" \\
..
FNORD

use_defaults_for_remaining_targets("cd build");
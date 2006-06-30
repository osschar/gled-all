#!/usr/bin/perl
# $Header$

# Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;

# Make-Dict-depend
##################
# Input: Header files that are to be rootcint-ed
# Output: make dependencies for generation of dictionary
#         usually stored in make_dict.inc
# first argument is dirname where dict files reside

# read build config
Gled_ConfCat_Parser::import_build_config();

while($hname=shift) {
  ($dir, $base) = $hname =~ m!^(\w+)/(\w+)!;
  $dbase = "$config->{DICT_DIR}/$base";
  print "$dbase.cc $dbase.h:\t$hname\n";
  print "\t\${ECHO} -=- Rootcint \$<\n";
  print "\t\${MUTE} (CPPFLAGS=\"\${CPPFLAGS}\" \${MKDICTGEN} $hname)\n";
}

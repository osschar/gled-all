#!/usr/bin/perl
# $Header$

# Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;

# Make-Blessed-depend
##################
# Input: none ... takes information from the build environment
# Output: make dependencies for blessed classes and their views +
#   definition of some make-file variables
#   stored in make_bless.inc and make_view.inc

# read build config
Gled_ConfCat_Parser::import_build_config();
Gled_ConfCat_Parser::parse_catalog();

open B, ">make_bless.inc";
open V, ">make_view.inc";

for $class (@{$CATALOG->{ClassList}}) {
  my $info = $CATALOG->{Classes}{$class};
  print B "$info->{Stem}.h7 $info->{Stem}.c7: $info->{Stem}.h\n";
  print B "\t\${ECHO} -7- Project7 blessed base \$<\n";
  print B "\t\${MUTE} \${P7} -c $class -i $info->{Stem}.h -1 $info->{Stem}.h7 ".
          "-3 $info->{Stem}.c7\n";

  my $dbase = "$config->{VIEW_DIR}/${class}View";
  my $h = "$dbase.h";
  my $c = "$dbase.cxx";
  my $o = "$dbase.o";
  push @hdrs, $h; push @srcs, $c; push @objs, $o;
  print V "$c $h:\t$info->{Stem}.h\n";
  print V "\t\${ECHO} -7- Project7 blessed view \$<\n";
  print V "\t\${MUTE} \${P7} -c $class -i $info->{Stem}.h -5 $h -7 $c\n";
}
close B;
print V "VIEW_AUTO_HDRS\t= ".join(' ', @hdrs)."\n";
print V "VIEW_AUTO_SRCS\t= ".join(' ', @srcs)."\n";
print V "VIEW_AUTO_OBJS\t= ".join(' ', @objs)."\n";
print V ".SECONDARY:\t\${VIEW_AUTO_HDRS} \${VIEW_AUTO_SRCS} ".
        "\${VIEW_AUTO_OBJS}\n";
close V;

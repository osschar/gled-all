#!/usr/bin/perl
# $Header$

# Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

# Greps header files for occurences of '#include "<sth>.h7"' directives.
# <sth> is taken as a class name AND file stem
# Spits deps but only generates the h7 files (no c7)

while($f=shift) {
  open F, "$f" or die "can't open $f";
  $f =~ m!(?:\./)?(.*)/([\w]+)\.h!;
  my $dir = $1;
  my $file_stem = "$1/$2";
  while($_=<F>) {
    if(/^\s*\#include\s+\"(\w+)\.h7\"/) {
      print "$file_stem.o: $dir/$1.h7\n";
      print "$dir/$1.h7: $file_stem.h\n";
      print "\t\${P7} -c $1 -i $f -1 $dir/$1.h7\n";
      push @hdrs, "$dir/$1.h7";
      push @srcs, "$dir/$1.c7";
    }
  }
  close F;
}
print "\n";
print "P7_AUTO_HDRS\t= ".join(' ', @hdrs)."\n";
print ".SECONDARY:\t\${P7_AUTO_HDRS}\n";

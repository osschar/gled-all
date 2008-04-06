#!/usr/bin/perl
# $Header$

# Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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
      my $class  = $1;
      my $h7file = "$dir/$class.h7";
      print "$file_stem.o: $h7file\n";
      print "$h7file: $file_stem.h\n";
      print "\t\${ECHO} -7- Project7 \$<\n";
      print "\t\${MUTE} \${P7} -c $class -i $f -1 $h7file\n";
      push @hdrs, $h7file;
    }
  }
  close F;
}
print "\n";
print "P7_AUTO_HDRS\t= ".join(' ', @hdrs)."\n";
print ".SECONDARY:\t\${P7_AUTO_HDRS}\n";

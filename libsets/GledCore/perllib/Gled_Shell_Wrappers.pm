# $Id$

# Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

# Not a proper module ... REQUIRE it
# $DRYRUN: report what would have been done

use Carp;

sub exec_or_die {
  my $cmd = shift;
  if($DRYRUN) {
    print "EXEC: $cmd\n";
  } else {
    my $ret = `$cmd`;
    croak "$cmd died" if $?;
    return $ret
  }
}

sub system_or_die {
  my $cmd = shift;
  if($DRYRUN) {
    print "SYST: $cmd\n";
  } else {
    system "$cmd" and die "$cmd died";
  }    
}

sub cd {
  my $dir = shift;
  if($DRYRUN) {
    print "CD: $dir\n";
    return 1;
  } else {
    return chdir $dir;
  }
}

1;

#!/usr/bin/perl -w
# $Header$

# Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

package Gled_ConfCat_Parser;

use Carp;

$CFGFILE     = "$ENV{GLEDSYS}/build_config";

sub import_build_config {
  # imports $GLEDSYS/build_config
  # exports the two hash-refs as $config and $resolver into main::
  croak "config file $CFGFILE not found (configedp?)"
    unless -e "$CFGFILE";
  do "$CFGFILE" or 
    croak"error evaling $CFGFILE";
  $main::config = $VAR1; $main::resolver = $VAR2;
}

sub parse_catalog {
  # Reads in catalog.auto and patches it w/ catalog.patch
  # Expects cwd to be the <LibSet> in question; 1st arg can be prefix
  # Returns it in global $CATALOG
  my $pref = shift;
  my $catalog = (defined $pref ? "$pref/" : "") . $main::config->{CATALOG};
  do "${catalog}.auto" or croak "can't open ${catalog}.auto";
  $main::CATALOG = $VAR1; # was plagued by Attempt to free unrefed scalar ... 
  return unless -r "${catalog}.patch";
  do "${catalog}.patch" or die "can't open ${catalog}.patch";
  # Now check for redefines
  for $k (keys %{ $PATCH }) {
    # print "Checking $k ...\n";
    for $i (keys %{ $PATCH->{$k} }) {
      # print "\tAppending $i => $PATCH->{$k}{$i} to CATALOG->{Classes}\n";
      $main::CATALOG->{Classes}{$k}{$i} = $PATCH->{$k}{$i};
    }
  }
}

1;

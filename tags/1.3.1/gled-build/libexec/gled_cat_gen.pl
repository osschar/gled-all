#!/usr/bin/perl
# $Id$

# Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

# Reads build_config and glass.list and writes out catalog.auto.
# This file is later read-in and patched with catalog.patch.
#
# Usage:
# gled_cat_gen.pl <libsetname> <libsetid>
 
use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;
use Data::Dumper;

$libname = shift;
$libid   = shift;

# Read-in $config and $resolver hash-refs

Gled_ConfCat_Parser::import_build_config();

########################################################################
# catalog.auto
########################################################################

$FAFALOG = { 'LibID' => $libid, 'LibSetName' => $libname };
open ZOO, $config->{GLASS_LIST} or die "can't open $config->{GLASS_LIST} for reading";
while ($_=<ZOO>)
{
  next if /^\#/ or /^\s*$/;
  chomp;
  s/\s*$//;
  my ($c, $id, $stem, $rnr) = split;
  $rnr = "" unless defined $rnr;
  $rnr = $c if $rnr eq ".";
  $FAFALOG->{Classes}{$c} =
  {
    'ClassID'    => $id,
    'PragmaSuff' => '+',
    'Stem'       => $stem,
    'RnrClass'   => $rnr
  };

  $FAFALOG->{ClassID2Name}{$id} = "$c";
  push @{$FAFALOG->{ClassList}}, $c;
}
close ZOO;

open O, ">$config->{CATALOG}.auto";
$Data::Dumper::Indent = 1;
$Data::Dumper::Purity = 1;
print O Dumper($FAFALOG);
close O;

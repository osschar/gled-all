#!/usr/bin/perl
# $Header$

# Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;
use Data::Dumper;

$libname = shift;
$libid = shift;

# produces $config and $resolver hash-refs
Gled_ConfCat_Parser::import_build_config();

# Create
unless(-e $config->{GLASS_LIST}) {
  my $dudedirs = join(" ", @ARGV);
  my @dudes = `find $dudedirs -name \\*.h`;
  map { chomp $_; $_ } @dudess;
  open ZOO, ">$config->{GLASS_LIST}" or die "can't open $config->{GLASS_LIST} for writing";
  my $i = 1;
  for $z (@dudess) {
    $z =~ m!^(.*)/(\w+)\.h$!o;
    my $tab = "\t" x (2 - int((length($2) / 8)));
    print ZOO "$2${tab}$i\t$1/$2\n";
    $i++;
  }
  close ZOO;
}

########################################################################
# catalog.auto
########################################################################

$FAFALOG = { 'LibID' => $libid, 'LibSetName' => $libname };
open ZOO, $config->{GLASS_LIST} or die "can't open $config->{GLASS_LIST} for reading";
while($_=<ZOO>) {
  next if /^\#/ or /^\s*$/;
  chomp; my ($c, $id, $stem, $rnr) = split;
  $rnr = "ZGlass" unless defined $rnr;
  $rnr = $c if $rnr eq ".";
  $FAFALOG->{Classes}{$c} = { 'ClassID' => $id, 'PragmaSuff' => '!',
			      'Stem' => $stem, 'RnrClass' => $rnr
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

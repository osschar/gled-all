#!/usr/bin/perl

# Loads GLED build configuration, reports stuff.

# !! In case of error the first char of output line is '<' !!

use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;

Gled_ConfCat_Parser::import_build_config();

my $command = shift;
die "<usage: $0 <command> <command arguments>>\n" unless defined $command;

if($command eq "lid2libname") {
  my $lid = shift;
  die "<usage: $0 $command <LibID>>\n" unless defined $lid;
  if(exists $resolver->{LibID2LibSetName}{$lid}) {
    print $resolver->{LibID2LibSetName}{$lid} . "\n";
  } else {
    print STDERR "$0: LibSet $lid not found!\n";
    exit 1;
  }
}

elsif($command eq "cppflags") {
  my $lsname = shift;
  die "<usage: $0 $command <LibSetName>>\n" unless defined $lsname;
  if( exists $resolver->{LibName2LibSpecs}{$lsname}{CPP_FLAGS} ) {
    print $resolver->{LibName2LibSpecs}{$lsname}{CPP_FLAGS} . "\n";
  } else {
    print STDERR "LibSet $lsname not found!\n";
    exit 1;
  }
}

elsif($command eq "libsets") {
  print join(" ", @{$resolver->{LibSetList}}) . "\n";
}

else {
  die "<unknown command $command>\n";
}

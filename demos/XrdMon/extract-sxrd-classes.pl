#!/usr/bin/perl

sub get_file
{
  my $fn = shift;
  open F, $fn or die "Cannot open $fn";

  local $/ = undef;
  my $f = <F>;
  close F;

  return $f;
}

#### main ####

print <<"FNORD";
Gathering all SXrd classes and their dependecies into a minimal set of files.
Note that SXrdClasses_LinkDef.h is maintained manually.
FNORD

open SXH, ">SXrdClasses.h"   or die "Cannot open output h file.";
open SXC, ">SXrdClasses.cxx" or die "Cannot open output cxx file.";

my $f;
my $pre = "//" . '=' x 78 . "\n";
my $pst = "\n\n";

print SXH get_file("sxrd-classes.h");

#### SXH ####

# SRange ... nogetsets, private to public
$f = get_file("$ENV{GLEDSYS}/GledCore/Stones/SRange.h");
$f =~ s/private:/public:/og;
$f =~ s/protected:/public:/og;
$f =~ s/^\#include.*SRange.h7.*$//om;
print SXH $pre, $f, $pst;

$f = get_file("$ENV{GLEDSYS}/XrdMon/Stones/SXrdServerInfo.h");
$f =~ s!^\s+void\s+Assign\(.*$!!omg;
print SXH $pre, $f, $pst;

$f = get_file("$ENV{GLEDSYS}/XrdMon/Stones/SXrdUserInfo.h");
$f =~ s!^\s+void\s+Assign\(.*$!!omg;
print SXH $pre, $f, $pst;

$f = get_file("$ENV{GLEDSYS}/XrdMon/Stones/SXrdFileInfo.h");
$f =~ s!^\s+void\s+Assign\(.*$!!omg;
$f =~ s!^\#include.*Stones/SRange.h.*$!!omg;
print SXH $pre, $f, $pst;

$f = get_file("$ENV{GLEDSYS}/XrdMon/Stones/SXrdIoInfo.h");
print SXH $pre, $f;

close SXH;

#### SXC ####

print SXC get_file("sxrd-classes.cxx");

$f = get_file("$ENV{GLEDSYS}/GledCore/Stones/SRange.cxx");
$f =~ s!(^\#include)!//$1!omg;
print SXC $pre, $f, $pst;

$f = get_file("$ENV{GLEDSYS}/XrdMon/Stones/SXrdIoInfo.cxx");
$f =~ s!(^\#include)!//$1!omg;
print SXC $pre, $f;

close SXC;

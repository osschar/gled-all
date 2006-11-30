#!/usr/bin/perl
# $Header$

# Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

# arguments ... files to generate dictionary from
# Safr ... if file name ends w/ NS ... it is considered a namespace

use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;

Gled_ConfCat_Parser::import_build_config();
Gled_ConfCat_Parser::parse_catalog();

my $dict = $config->{DICT_DIR};

while($_=shift) {
  ($cname) = m!/(\w+).h$!;
  # $cname is base name of the file and by default also class name

  my $pragma_default;
  my $pragma_link;

  if($CATALOG->{Classes}{$cname}{NoDefault} == 0) {

    my $pragma_suff;
    if(defined $CATALOG->{Classes}{$cname}{PragmaSuff}) {
      $pragma_suff = $CATALOG->{Classes}{$cname}{PragmaSuff};
    } else {
      $pragma_suff = "+";
    }
    if(defined $resolver->{GlassName2GlassSpecs}{$cname}) {
      $pragma_link = "#pragma link C++ class ZLink<${cname}>;";
    }
    
    my $token = ($cname =~ m/NS$/) ? 'namespace' : 'class';

    $pragma_default = "#pragma link C++ ${token} ${cname}${pragma_suff};";

  }
  open( FOO, ">$dict/${cname}_LinkDef.h");
  print FOO <<"END";
#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;
#pragma link C++ nestedenum;
#pragma link C++ nestedtypedef;

$CATALOG->{Classes}{$cname}{PrePragmas}
${pragma_link}
${pragma_default}
$CATALOG->{Classes}{$cname}{Pragmas}
$CATALOG->{Classes}{$cname}{PostPragmas}

#endif
END
  close FOO;
  my $cintopts = $CATALOG->{Classes}{$cname}{CINT_Opts};
  $exe = "rootcint -f $dict/$cname.cc -c -p $cintopts " .
         "-I. $ENV{CPPFLAGS} -I$ENV{ROOTSYS}/include " .
	 "$_ $dict/${cname}_LinkDef.h";
  # print $exe."\n";
  my $ret = `$exe`;
  croak $ret if $?;
}
# Remove all traces
#unlink "${cname}_LinkDef.h";

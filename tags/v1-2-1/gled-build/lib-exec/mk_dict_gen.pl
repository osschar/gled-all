#!/usr/bin/perl
# $Header$

# Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

# arguments ... files to generate dictionary from
# Safr ... if file name ends w/ NS ... it is considered a namespace

use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;

Gled_ConfCat_Parser::import_build_config();
Gled_ConfCat_Parser::parse_catalog("../");

while($_=shift) {
  ($cname) = m!/(\w+).h$!;
  # $cname is now base name ... also class name
  print "Messing $_ $cname, suff=",$CATALOG->{Classes}{$cname}{PragmaSuff},"\n";

  $token = ($cname =~ m/NS$/) ? 'namespace' : 'class';
  #$token = 'class';

  open( FOO, ">${cname}_LinkDef.h");
  print FOO <<"END";
#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ ${token} ${cname}$CATALOG->{Classes}{$cname}{PragmaSuff};
$CATALOG->{Classes}{$cname}{Pragmas}
#endif
END
  close FOO;
  my $cintopts = $CATALOG->{Classes}{$cname}{CINT_Opts};
  # when do i need the -p option ... what includes ...
  # on general ... il ne faudrait pas savoir de gl dans les .h datoquoques
  $exe = "rootcint -f $cname.cc -c $cintopts " .
         "-I.. $ENV{CPPFLAGS} -I$ENV{ROOTSYS}/include " .
	 "../$_ ${cname}_LinkDef.h";
  print $exe."\n";
  `$exe`;
  exit($_) if($?)
}
# Remove all traces
#unlink "${cname}_LinkDef.h";


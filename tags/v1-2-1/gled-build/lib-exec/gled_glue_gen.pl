#!/usr/bin/perl
# $Header$

# Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;

Gled_ConfCat_Parser::import_build_config();

arl: {
  $_ = shift;
  last arl if $_ eq '::';
  push @Deps, $_;
  redo arl;
} 

@Rnrs = @ARGV;

Gled_ConfCat_Parser::parse_catalog();
$libname = $CATALOG->{LibSetName};
$libid = $CATALOG->{LibID};

########################################################################
# LibSet
########################################################################

open H, ">$config->{GLUE_DIR}/${libname}_LibSet.h";
print H <<"fnord";
#ifndef GLED_${libname}LibSet
#define GLED_${libname}LibSet

void   lib${libname}_GLED_init();
#endif
fnord
close H;

open C, ">$config->{GLUE_DIR}/${libname}_LibSet.cxx";
print C <<"fnord";
#include "${libname}_LibSet.h"
#include <Gled/GledNS.h>
#include <Stones/ZMIR.h>
#include <Ephra/Saturn.h>
#include <TBuffer.h>

fnord

# includes and static arrays

for $c (@{ $CATALOG->{ClassList} }) {
  print C "#include <$CATALOG->{Classes}{$c}{Stem}.h>\n";
}

{
  my $xx = ($#Deps>-1) ? "\"".join("\", \"",@Deps)."\"," : "";
  print C "\nstatic char* _deplist[] = { $xx 0 };\n\n";
}

# E_Demangle
print C <<"fnord";
Int_t
lib${libname}_E_Demangle(ZGlass *n, TBuffer* b) {
  CID_t classId; *b >> classId;
  switch(classId) {
fnord

for $c (@{ $CATALOG->{ClassList} }) {
  print C <<"fnord";
  case $CATALOG->{Classes}{$c}{ClassID}: {
    $c* _true_node = dynamic_cast<$c*>( n );
    if(!_true_node) return 1;
    return _true_node->E_Exec(b);
  }
fnord
}

print C <<"fnord";
  default: {
    return 16;
  }
  } // switch
}
fnord

# E_Construct
print C <<"fnord";
ZGlass*
lib${libname}_E_Construct(Saturn* s, TBuffer* b) {
  CID_t classId; *b >> classId;
  switch(classId) {
fnord
for $c (@{ $CATALOG->{ClassList} }) {
  print C "  case $CATALOG->{Classes}{$c}{ClassID}: { return " .
  (($CATALOG->{Classes}{$c}{VirtualBase}) ? "0" : "${c}::Btor(s, b)") . "; }\n";
}
print C <<"fnord";
  default: { return 0; }
  } // switch
}
fnord

# DEF_Construct
print C <<"fnord";
ZGlass*
lib${libname}_DEF_Construct(CID_t cid) {
  switch(cid) {
fnord
for $c (@{ $CATALOG->{ClassList} }) {
  print C "  case $CATALOG->{Classes}{$c}{ClassID}: { return " .
  (($CATALOG->{Classes}{$c}{VirtualBase}) ? "0" : "new ${c}") . "; }\n";
}
print C <<"fnord";
  default: { return 0; }
  } // switch
}
fnord

# IS_A
print C <<"fnord";
bool lib${libname}_IS_A(ZGlass* g, CID_t cid) {
  switch(cid) {
fnord

for $c (@{ $CATALOG->{ClassList} }) {
  print C <<"fnord";
  case $CATALOG->{Classes}{$c}{ClassID}: {
    return (dynamic_cast<$c*>( g ) != 0);
  }
fnord
}

print C <<"fnord";
  default: {
    // well safertundhell ... this monkeys should all be exception throwing
    return false;
  }
  } // switch
}
fnord


# GLED_init
print C <<"fnord";
void
lib${libname}_GLED_init() {
  GledNS::LibSetInfo* lsi = new GledNS::LibSetInfo;
  lsi->fLid = $libid;
  lsi->fName = "$libname";
  lsi->fDeps = _deplist;
  lsi->fEDFoo = lib${libname}_E_Demangle;
  lsi->fECFoo = lib${libname}_E_Construct;
  lsi->fDCFoo = lib${libname}_DEF_Construct;
  lsi->fISAFoo = lib${libname}_IS_A;
  GledNS::BootstrapSoSet(lsi);
fnord
for $c (@{ $CATALOG->{ClassList} }) {
  print C "  GledNS::BootstrapClass(\"$c\", $libid, $CATALOG->{Classes}{$c}{ClassID});\n";
}
print C "}\n\n";
print C "void *${libname}_GLED_init = (void*)lib${libname}_GLED_init;\n";
close C;

########################################################################
# View stuff ...
########################################################################

open H, ">$config->{GLUE_DIR}/${libname}_View_LibSet.h";
print H <<"fnord";
#ifndef GLED_${libname}ViewLibSet
#define GLED_${libname}ViewLibSet

void lib${libname}_GLED_init_View();

#endif
fnord

close H;

open C, ">$config->{GLUE_DIR}/${libname}_View_LibSet.cxx";
print C <<"fnord";
#include "${libname}_View_LibSet.h"
#include <GledView/GledViewNS.h>
#include <Glasses/ZGlass.h>

fnord

for $c (@{ $CATALOG->{ClassList} }) {
  print C "#include <$config->{VIEW_DIR}/${c}View.h>\n"
}
print C "\n";

print C <<"fnord";
void lib${libname}_GLED_init_View() {
  GledViewNS::BootstrapViewSet($libid, "$libname");
fnord
for $c (@{ $CATALOG->{ClassList} }) {
  print C <<"fnord";
  ${c}View::CheckIn();
fnord
}
print C "}\n\n";
print C "void *${libname}_GLED_init_View = (void*)lib${libname}_GLED_init_View;\n";

# !! should also generate _fini to clean up LibSet/Class/MemberInfo's

close C;

########################################################################
# Rnrs
########################################################################

%existing_rnrs;
for $c (@{ $CATALOG->{ClassList} }) {
  my $rnr_class = $CATALOG->{Classes}{$c}{RnrClass};
  $existing_rnrs{$rnr_class} = 1 if exists $CATALOG->{Classes}{$rnr_class};
}

for $rnr (@Rnrs) {

open H, ">$config->{GLUE_DIR}/${libname}_Rnr_${rnr}_LibSet.h";
print H <<"fnord";
#ifndef GLED_${libname}_Rnr_${rnr}_LibSet
#define GLED_${libname}_Rnr_${rnr}_LibSet

void lib${libname}_GLED_init_Rnr_${rnr}();

#endif
fnord

close H;

open C, ">$config->{GLUE_DIR}/${libname}_Rnr_${rnr}_LibSet.cxx";
print C <<"fnord";
#include "${libname}_Rnr_${rnr}_LibSet.h"
#include <GledView/GledViewNS.h>
#include <Glasses/ZGlass.h>

fnord

for $r (keys %existing_rnrs) {
  print C "#include <Rnr/${rnr}/${r}_${rnr}_Rnr.h>\n";
}

print C <<"fnord";

A_Rnr*
lib${libname}_Rnr_${rnr}_Construct(ZGlass* n, CID_t cid) {
  A_Rnr* v=0;
  switch(cid) {
fnord

for $c (keys %existing_rnrs) {
  print C <<"fnord";
  case $CATALOG->{Classes}{$c}{ClassID}: {
    v = new ${c}_${rnr}_Rnr(dynamic_cast<$c*>(n));
    break;
  }
fnord
}

print C <<"fnord";
  } // switch
  return v;
}

void lib${libname}_GLED_init_Rnr_${rnr}() {
  GledViewNS::BootstrapRnrSet("$libname", $libid, "$rnr", lib${libname}_Rnr_${rnr}_Construct);
}

void *${libname}_GLED_init_Rnr_${rnr} = (void*)lib${libname}_GLED_init_Rnr_${rnr};
fnord

close C;
  
}

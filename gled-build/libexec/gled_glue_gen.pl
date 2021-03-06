#!/usr/bin/perl
# $Id$

# Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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

fnord

# includes and static arrays

for $c (@{ $CATALOG->{ClassList} }) {
  print C "#include <$CATALOG->{Classes}{$c}{Stem}.h>\n";
}

{
  my $xx = ($#Deps>-1) ? "\"".join("\", \"",@Deps)."\"," : "";
  print C "\nstatic const char* _deplist[] = { $xx 0 };\n\n";
}

# E_Demangle
print C <<"fnord";
void lib${libname}_Mir_Exec(ZGlass *lens, ZMIR& mir) {
  static const string _eh("lib${libname}_Mir_Exec dynamic_cast to ");
  switch(mir.fCid) {
fnord

for $c (@{ $CATALOG->{ClassList} }) {
  print C <<"fnord";
  case $CATALOG->{Classes}{$c}{ClassID}: {
    $c* _true_node = dynamic_cast<$c*>(lens);
    if(!_true_node) throw(_eh + "glass '$c' failed.");
    _true_node->ExecuteMir(mir);
    break;
  }
fnord
}

print C <<"fnord";
  default: { throw(_eh + "unknown ClassID."); }
  } // switch
}
fnord

# libset_Lens_Constructor
print C <<"fnord";
ZGlass* lib${libname}_Lens_Constructor(CID_t cid) {
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

# Is_A_Glass
print C <<"fnord";
bool lib${libname}_Is_A_Glass(ZGlass* g, CID_t cid) {
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
  default: { return false; }
  } // switch
}
fnord


# GLED_init
print C <<"fnord";
void
lib${libname}_GLED_init() {
  GledNS::LibSetInfo* lsi = new GledNS::LibSetInfo("$libname", $libid);
  lsi->fDeps = _deplist;
  lsi->fLME_Foo = lib${libname}_Mir_Exec;
  lsi->fLC_Foo = lib${libname}_Lens_Constructor;
  lsi->fISA_Foo = lib${libname}_Is_A_Glass;
  GledNS::BootstrapSoSet(lsi);
fnord
for $c (@{ $CATALOG->{ClassList} }) {
  print C "  ${c}::_gled_catalog_init();\n";
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
  ${c}View::_gled_catalog_init();
fnord
}
print C "}\n\n";
print C "void *${libname}_GLED_init_View = (void*)lib${libname}_GLED_init_View;\n";

# !! should also generate _fini to clean up LibSet/Class/MemberInfo's

close C;


########################################################################
# Rnrs
########################################################################

@class_rnrs = ();

for $c (@{$CATALOG->{ClassList}})
{
  if ($CATALOG->{Classes}{$c}{RnrClass} eq $c)
  {
    push @class_rnrs, $c;
  }
}

for $rnr (@Rnrs)
{
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
#include <Gled/GledNS.h>
#include <Glasses/ZGlass.h>

fnord

  for $c (@class_rnrs)
  {
    print C "#include <Rnr/${rnr}/${c}_${rnr}_Rnr.h>\n";
  }

  print C <<"fnord";

A_Rnr* lib${libname}_Rnr_${rnr}_Construct(ZGlass* n, CID_t cid)
{
  A_Rnr* v=0;
  switch(cid) {
fnord

  for $c (@class_rnrs)
  {
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

void lib${libname}_GLED_init_Rnr_${rnr}()
{
  GledNS::BootstrapRnrSet("$libname", $libid, "$rnr", lib${libname}_Rnr_${rnr}_Construct);
}

void *${libname}_GLED_init_Rnr_${rnr} = (void*)lib${libname}_GLED_init_Rnr_${rnr};
fnord

  close C;

} # end for @Rnrs

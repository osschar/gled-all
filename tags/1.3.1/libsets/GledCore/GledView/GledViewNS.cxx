// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GledViewNS.h"
#include <Gled/GledNS.h>
#include <Glasses/ZGlass.h>
#include <TSystem.h>

#include <algorithm>
#include <iterator>

namespace GledViewNS {
  int no_symbol_label = 0;
  int menubar_box     = 0;
} // GledViewNS

namespace GVNS = GledViewNS;

/**************************************************************************/
// These two as in GledNS ... but for Views

Int_t GledViewNS::LoadSoSet(const TString& lib_set)
{
  TString libname = FabricateViewLibName(lib_set);
  Int_t ret = GledNS::LoadSo(libname.Data());
  if(ret) {
    ISmess(GForm("GledViewNS::LoadSoSet loading %s as %s returned %d",
		 lib_set.Data(), libname.Data(), ret));
  }
  if(ret < 0) return ret;
  return InitSoSet(lib_set);
}

Int_t GledViewNS::InitSoSet(const TString& lib_set)
{
  { // init
    TString cmd = FabricateViewInitFoo(lib_set);
    long* p2foo = (long*) GledNS::FindSymbol(cmd);
    if(!p2foo) {
      ISerr(GForm("GledViewNS::InitSoSet can't find %s. Safr!",
		  cmd.Data()));
      return 2;
    }
    void (*foo)() = (void(*)())(*p2foo);
    foo();
  }
  { // user init
    TString cmd = FabricateViewUserInitFoo(lib_set);
    long* p2foo = (long*) GledNS::FindSymbol(cmd);
    if(!p2foo) {
      ISmess(GForm("GledViewNS::InitSoSet no user initialization for %s.",
		   lib_set.Data()));
    } else {
      ISmess(GForm("GledViewNS::InitSoSet execing user initialization for %s.",
		   lib_set.Data()));
      void (*foo)() = (void(*)())(*p2foo);
      foo();
    }
  }
  return 0;
}

/**************************************************************************/

void GledViewNS::BootstrapViewSet(LID_t lid, const TString& libset)
{
  GledNS::LibSetInfo* gns_lsi = GledNS::FindLibSetInfo(lid);
  if(gns_lsi == 0) {
    ISerr(GForm("GledViewNS::BootstrapViewSet %s(id=%u) not loaded ...",
		libset.Data(), lid));
    return;
  }
  if(gns_lsi->fViewPart != 0) {
    ISwarn(GForm("GledViewNS::BootstrapViewSet %s(id=%u) already loaded ...",
		 libset.Data(), lid));
    return;
  }
  ISmess(GForm("GledViewNS::BootstrapViewSet installing %s(id=%u) ...",
	       libset.Data(), lid));

  gns_lsi->fViewPart = new LibSetInfo();
  // Init deps as well ... loaded by link-time dependence
  const char** dep = gns_lsi->fDeps;
  while(*dep) {
    GledNS::LibSetInfo* dlsi = GledNS::FindLibSetInfo(*dep);
    if(dlsi == 0) {
      ISerr("GledViewNS::BootstrapViewSet dependent libset not initialised. Aborting.");
      return;
    }
    if(dlsi->fViewPart == 0) {
      Int_t ini = InitSoSet(*dep);
      if(ini) {
	ISerr("GledViewNS::BootstrapViewSet initialisation of dependency failed. Aborting.");
	return;
      }
    }
    ++dep;
  }
}

void GledViewNS::BootstrapClassInfo(ClassInfo* c_info)
{
  // This actually does nothing now ...
}

/**************************************************************************/

TString GledViewNS::FabricateViewLibName(const TString& libset)
{
  return TString("lib") + libset + "_View.so";
}

TString GledViewNS::FabricateViewInitFoo(const TString& libset)
{
  TString foo = libset + "_GLED_init_View";
  return foo;
}

TString GledViewNS::FabricateViewUserInitFoo(const TString& libset)
{
  TString foo = libset + "_GLED_user_init_View";
  return foo;
}

/**************************************************************************/
/**************************************************************************/
// Sub structs
/**************************************************************************/
/**************************************************************************/

namespace {
  struct infobase_name_eq : public unary_function<GledNS::InfoBase*, bool> {
    TString name;
    infobase_name_eq(const TString& s) : name(s) {}
    bool operator()(const GledNS::InfoBase* ib) {
      return ib->fName == name; }
  };
}

GledViewNS::WeedInfo*
GledViewNS::ClassInfo::FindWeedInfo(const TString& name, bool recurse,
				    GledNS::ClassInfo* true_class)
{
  lpWeedInfo_i i = find_if(fWeedList.begin(), fWeedList.end(),
			   infobase_name_eq(name));
  if(i != fWeedList.end()) return *i;
  if(recurse && true_class != 0) {
    GledNS::ClassInfo* p = true_class->GetParentCI();
    if(p) return p->fViewPart->FindWeedInfo(name, recurse, p);
  }
  return 0;
}

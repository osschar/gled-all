// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GledViewNS.h"
#include <Gled/GledNS.h>
#include <Glasses/ZGlass.h>
#include <TSystem.h>

#include <algorithm>
#include <iterator>

namespace GledViewNS {
  set<string>	RnrNames;
  list<MTW_Layout_Spec> mtw_layouts;
  int no_symbol_label = 0;
  int menubar_box     = 0;
} // GledViewNS

namespace GVNS = GledViewNS;

/**************************************************************************/
// These two as in GledNS ... but for Views
#include <G__ci.h>

Int_t GledViewNS::LoadSoSet(const string& lib_set)
{
  string libname = FabricateViewLibName(lib_set);
  Int_t ret = GledNS::LoadSo(libname.c_str());
  if(ret) {
    ISmess(GForm("GledViewNS::LoadSoSet loading %s as %s returned %d",
		 lib_set.c_str(), libname.c_str(), ret));
  }
  if(ret < 0) return ret;
  ret = InitSoSet(lib_set);
  if(ret) return ret;
  AssertRenderers();
  return ret;
}

Int_t GledViewNS::InitSoSet(const string& lib_set)
{
  { // init
    string cmd = FabricateViewInitFoo(lib_set);
    long* p2foo = (long*) G__findsym( cmd.c_str() );
    if(!p2foo) {
      ISerr(GForm("GledViewNS::InitSoSet can't find %s. Safr!",
		  cmd.c_str()));
      return 2;
    }
    void (*foo)() = (void(*)())(*p2foo);
    foo();
  }
  { // user init
    string cmd = FabricateViewUserInitFoo(lib_set);
    long* p2foo = (long*) G__findsym( cmd.c_str() );
    if(!p2foo) {
      ISmess(GForm("GledViewNS::InitSoSet no user initialization for %s.",
		   lib_set.c_str()));
    } else {
      ISmess(GForm("GledViewNS::InitSoSet execing user initialization for %s.",
		   lib_set.c_str()));
      void (*foo)() = (void(*)())(*p2foo);
      foo();
    }
  }
  return 0;
}

/**************************************************************************/

void GledViewNS::BootstrapViewSet(LID_t lid, const string& libset)
{
  GledNS::LibSetInfo* gns_lsi = GledNS::FindLibSetInfo(lid);
  if(gns_lsi == 0) {
    ISerr(GForm("GledViewNS::BootstrapViewSet %s(id=%u) not loaded ...",
		libset.c_str(), lid));
    return;
  }
  if(gns_lsi->fViewPart != 0) {
    ISwarn(GForm("GledViewNS::BootstrapViewSet %s(id=%u) already loaded ...",
		 libset.c_str(), lid));
    return;
  }
  ISmess(GForm("GledViewNS::BootstrapViewSet installing %s(id=%u) ...",
	       libset.c_str(), lid));

  gns_lsi->fViewPart = new LibSetInfo();
  // Init deps as well ... loaded by link-time dependence
  char** dep = gns_lsi->fDeps;
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
  // The glass's bootstrap method needs to know master ClassInfo anyway.
  /*
  FID_t& fid = c_info->fFid;
  hLid2LSInfo_i i = Lid2LSInfo.find(fid.lid);
  if(i == Lid2LSInfo.end()) {
    ISwarn(GForm("GledViewNS::BootstrapClassView LibSet (lid=%u) not loaded ...",
		 fid.lid));
    return;
  }
  hCid2pCI_i j = i->second.Cid2pCI.find(fid.cid);
  if(j != i->second.Cid2pCI.end()) {
    ISwarn(GForm("GledViewNS::BootstrapClassView class id %u already checked in ...",
		 fid.cid));
    return;
  }
  i->second.Cid2pCI[fid.cid] = c_info;
  */
}

void GledViewNS::BootstrapRnrSet(const string& libset, LID_t lid,
				 const string& rnr, A_Rnr_Creator_foo rfoo)
{
  GledNS::LibSetInfo* gns_lsi = GledNS::FindLibSetInfo(lid);
  if(gns_lsi == 0) {
    ISwarn(GForm("GledViewNS::BootstrapRnrSet LibSet %s(lid=%u) not loaded ...",
		 libset.c_str(), lid));
    return;
  }
  hRnr2RCFoo_i j = gns_lsi->fViewPart->Rnr2RCFoo.find(rnr);
  if(j != gns_lsi->fViewPart->Rnr2RCFoo.end()) {
    ISwarn(GForm("GledViewNS::BootstrapRnrSet RnrCreator for rnr=%s, LibSet=%s (lid=%u) already present ...",
		 rnr.c_str(), libset.c_str(), lid));
    return;
  }
  gns_lsi->fViewPart->Rnr2RCFoo[rnr] = rfoo;
}

/**************************************************************************/

string GledViewNS::FabricateViewLibName(const string& libset)
{
  return string("lib") + libset + "_View.so";
}

string GledViewNS::FabricateViewInitFoo(const string& libset)
{
  string foo = libset + "_GLED_init_View";
  return foo;
}

string GledViewNS::FabricateViewUserInitFoo(const string& libset)
{
  string foo = libset + "_GLED_user_init_View";
  return foo;
}

string GledViewNS::FabricateRnrLibName(const string& libset, const string& rnr)
{
  return string("lib") + libset + "_Rnr_" + rnr + ".so";
}

string GledViewNS::FabricateRnrInitFoo(const string& libset, const string& rnr)
{
  string foo = libset + "_GLED_init_Rnr_" + rnr;
  return foo;
}


/**************************************************************************/

void GledViewNS::AssertRenderers()
{
  GledNS::lpLSI_t ls_list;
  GledNS::ProduceLibSetInfoList(ls_list);
  for(GledNS::lpLSI_i lsi=ls_list.begin(); lsi!=ls_list.end(); ++lsi) {
    string libset = (*lsi)->fName;
    for(set<string>::iterator rnr=RnrNames.begin(); rnr!=RnrNames.end(); ++rnr) {
      if((*lsi)->fViewPart->Rnr2RCFoo.find(*rnr) == (*lsi)->fViewPart->Rnr2RCFoo.end()) {
	string cmd = FabricateRnrInitFoo(libset, *rnr);
	long* p2foo = (long*) G__findsym( cmd.c_str() );
	if(!p2foo) {
	  string libname = FabricateRnrLibName(libset, *rnr);
	  int ret = GledNS::LoadSo(libname);
	  if(ret < 0) {
	    ISerr(GForm("GledViewNS::AssertRenderers %s not existing",
			libname.c_str()));
	    return;
	  }
	  p2foo = (long*) G__findsym( cmd.c_str() );
	  if(!p2foo) {
	    ISerr(GForm("GledViewNS::AssertRenderers %s not existing in %s",
			cmd.c_str(), libname.c_str()));
	    return;
	  }
	}
	void (*foo)() = (void(*)())(*p2foo);
	foo();
      }
    }
  }
}

void GledViewNS::AddRenderer(const string& rnr)
{
  if(RnrNames.find(rnr) == RnrNames.end()) {
    RnrNames.insert(rnr);
    AssertRenderers();
  }
}

/**************************************************************************/
/**************************************************************************/
// Services
/**************************************************************************/
/**************************************************************************/

A_Rnr* GledViewNS::SpawnRnr(const string& rnr, ZGlass* d, FID_t fid)
{
  GledNS::LibSetInfo* gns_lsi = GledNS::FindLibSetInfo(fid.lid);
  if(gns_lsi == 0) {
    ISerr(GForm("GledViewNS::SpawnRnr can't demangle lib id=%u", fid.lid));
    return 0;
  }
  hRnr2RCFoo_i j = gns_lsi->fViewPart->Rnr2RCFoo.find(rnr);
  if(j == gns_lsi->fViewPart->Rnr2RCFoo.end()) {
    ISerr(GForm("GledViewNS::SpawnRnr can't find Rnr Constructor for %s",
		rnr.c_str()));
    return 0;
  }
  return (j->second)(d, fid.cid);
}

/**************************************************************************/
/**************************************************************************/
// Sub structs
/**************************************************************************/
/**************************************************************************/

namespace {
  struct infobase_name_eq : public unary_function<GledNS::InfoBase*, bool> {
    string name;
    infobase_name_eq(const string& s) : name(s) {}
    bool operator()(const GledNS::InfoBase* ib) {
      return ib->fName == name; }
  };
}


GledViewNS::WeedInfo*
GledViewNS::ClassInfo::FindWeedInfo(const string& name, bool recurse, GledNS::ClassInfo* true_class)
{
  lpWeedInfo_i i = find_if(fWeedList.begin(), fWeedList.end(),
			   infobase_name_eq(name));
  if(i != fWeedList.end()) return *i;
  if(recurse) {
    GledNS::ClassInfo* p = true_class->GetParentCI();
    if(p) return p->fViewPart->FindWeedInfo(name, recurse, true_class);
  }
  return 0;
}

GledNS::ClassInfo* GledViewNS::ClassInfo::GetRendererCI()
{
  if(!fRendererCI && !fRendererGlass.empty()) {
    fRendererCI = GledNS::FindClassInfo(fRendererGlass);
  }
  return fRendererCI;
}

A_Rnr* GledViewNS::ClassInfo::SpawnRnr(const string& rnr, ZGlass* g)
{
  if(fRendererCI == 0) GetRendererCI();
  //cout <<"GledViewNS::ClassInfo::SpawnRnr rnr="<< rnr <<", lens="<< g->GetName() <<
  //"["<< fRendererCI->fClassName <<"]\n";
  return GledViewNS::SpawnRnr(rnr, g, fRendererCI->fFid);
}

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GledViewNS.h"
#include <Gled/GledNS.h>
#include <Glasses/ZGlass.h>
#include <TSystem.h>

#include <algorithm>
#include <iterator>

namespace GledViewNS {
  hLid2LSInfo_t	Lid2LSInfo;
  set<string>	RnrNames;
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
  hLid2LSInfo_i i = Lid2LSInfo.find(lid);
  if(i != Lid2LSInfo.end()) {
    ISwarn(GForm("GledViewNS::BootstrapViewSet %s(id=%u) already loaded ...",
		 libset.c_str(), lid));
    return;
  }
  ISmess(GForm("GledViewNS::BootstrapViewSet installing %s(id=%u) ...",
	       libset.c_str(), lid));
  Lid2LSInfo.insert(pair<LID_t, LibSetInfo>(lid, LibSetInfo(lid,libset)));
  // Init deps as well ... loaded by link-time dependence
  char** dep = GledNS::Lid2pLSI[lid]->fDeps;
  while(*dep) {
    if(GledNS::Name2Lid.find(*dep) == GledNS::Name2Lid.end()) {
      Int_t ini = InitSoSet(*dep);
      if(ini) {
	ISerr("GledViewNS::BootstrapViewSet aborting");
	return;
      }
    }
    ++dep;
  }
}

void GledViewNS::BootstrapClassInfo(ClassInfo* c_info)
{
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
}

void GledViewNS::BootstrapRnrSet(const string& libset, LID_t lid,
			    const string& rnr, A_Rnr_Creator_foo rfoo)
{
  hLid2LSInfo_i i = Lid2LSInfo.find(lid);
  if(i == Lid2LSInfo.end()) {
    ISwarn(GForm("GledViewNS::BootstrapRnrSet LibSet %s(lid=%u) not loaded ...",
		 libset.c_str(), lid));
    return;
  }
  hRnr2RCFoo_i j = i->second.Rnr2RCFoo.find(rnr);
  if(j != i->second.Rnr2RCFoo.end()) {
    ISwarn(GForm("GledViewNS::BootstrapRnrSet RnrCreator for rnr=%s, LibSet=%s (lid=%u) already present ...",
		 rnr.c_str(), libset.c_str(), lid));
    return;
  }
  i->second.Rnr2RCFoo[rnr] = rfoo;
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
  for(hLid2LSInfo_i lvrc=Lid2LSInfo.begin(); lvrc!=Lid2LSInfo.end(); ++lvrc) {
    string libset = lvrc->second.fName;
    for(set<string>::iterator rnr=RnrNames.begin(); rnr!=RnrNames.end(); ++rnr) {
      if(lvrc->second.Rnr2RCFoo.find(*rnr) == lvrc->second.Rnr2RCFoo.end()) {
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
// Inquiries
/**************************************************************************/

GledViewNS::LibSetInfo* GledViewNS::FindLibSetInfo(LID_t lid)
{
  hLid2LSInfo_i i = Lid2LSInfo.find(lid);
  if(i == GledViewNS::Lid2LSInfo.end()) {
    ISerr(GForm("GledViewNS::FindLibSetInfo can't demangle lib id=%u", lid));
    return 0;
  }
  return &(i->second);
}

GledViewNS::ClassInfo* GledViewNS::FindClassInfo(FID_t fid)
{
  if(fid.is_null()) return 0;
  hLid2LSInfo_i i = Lid2LSInfo.find(fid.lid);
  if(i == GledViewNS::Lid2LSInfo.end()) {
    ISerr(GForm("GledViewNS::FindClassInfo can't demangle lib id=%u", fid.lid));
    return 0;
  }
  return i->second.FindClassInfo(fid.cid);
}


/**************************************************************************/
/**************************************************************************/
// Services
/**************************************************************************/
/**************************************************************************/

A_Rnr* GledViewNS::SpawnRnr(const string& rnr, ZGlass* d, LID_t lid, CID_t cid)
{
  hLid2LSInfo_i i = Lid2LSInfo.find(lid);
  if(i == GledViewNS::Lid2LSInfo.end()) {
    ISerr(GForm("GledViewNS::SpawnRnr can't demangle lib id=%u", lid));
    return 0;
  }
  hRnr2RCFoo_i j = i->second.Rnr2RCFoo.find(rnr);
  if(j == i->second.Rnr2RCFoo.end()) {
    ISerr(GForm("GledViewNS::SpawnRnr can't find Rnr Constructor for %s",
		rnr.c_str()));
    return 0;
  }
  return (j->second)(d, cid);
}

/**************************************************************************/
/**************************************************************************/
// Sub structs
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// GledViewNS::LibSetInfo
/**************************************************************************/

GledViewNS::ClassInfo*
GledViewNS::LibSetInfo::FindClassInfo(CID_t cid)
{
  hCid2pCI_i i = Cid2pCI.find(cid);
  if(i == Cid2pCI.end()) {
    ISerr(GForm("GledViewNS::LibSetInfo::FindClassInfo can't demangle class cid=%u",
		cid));
    return 0;
  }
  return i->second;
}

GledViewNS::ClassInfo*
GledViewNS::LibSetInfo::FirstClassInfo()
{
  hCid2pCI_i i = Cid2pCI.begin();
  if(i == Cid2pCI.end()) {
    ISerr("GledViewNS::LibSetInfo::FirstClassInfo no classes found");
    return 0;
  }
  return i->second;
}

/**************************************************************************/
// GledViewNS::ClassInfo
/**************************************************************************/

GledViewNS::lpMemberInfo_t*
GledViewNS::ClassInfo::ProduceFullMemberInfoList()
{
  // Recursive up call towards the base (ZGlass)
  lpMemberInfo_t* ret;
  ClassInfo* p = GetParentCI();
  if(p) ret = p->ProduceFullMemberInfoList();
  else	ret = new lpMemberInfo_t;
  copy(fMIlist.begin(), fMIlist.end(), back_inserter(*ret));
  return ret;
}

GledViewNS::lpLinkMemberInfo_t*
GledViewNS::ClassInfo::ProduceFullLinkMemberInfoList()
{
  // Recursive up call towards the base (ZGlass)
  lpLinkMemberInfo_t* ret;
  ClassInfo* p = GetParentCI();
  if(p) ret = p->ProduceFullLinkMemberInfoList();
  else	ret = new lpLinkMemberInfo_t;
  copy(fLMIlist.begin(), fLMIlist.end(), back_inserter(*ret));
  return ret;
}

GledViewNS::MemberInfo* GledViewNS::ClassInfo::FindMemberInfo(const string& s)
{
  // !!!! should recurse into parent
  mName2pMemberInfo_i i = fMImap.find(s);
  if(i == fMImap.end()) {
    ISerr(GForm("GledViewNS::ClassInfo::FindMemberInfo can't demangle name %s",
		s.c_str()));
    return 0;
  }
  return i->second;
}

struct cmi_name_eq : public unary_function<GVNS::ContextMethodInfo*, bool> {
  string name;
  cmi_name_eq(const string& s) : name(s) {}
  bool operator()(const GVNS::ContextMethodInfo* cmi) {
    return cmi->fName == name; }
};

GledViewNS::ContextMethodInfo*
GledViewNS::ClassInfo::FindContextMethodInfo(const string& func_name)
{
  lpContextMethodInfo_i i = find_if(fCMIlist.begin(), fCMIlist.end(),
				    cmi_name_eq(func_name));
  if(i==fCMIlist.end()) {
    ClassInfo* p = GetParentCI();
    if(p) return p->FindContextMethodInfo(func_name);
    else  return 0;
  } else {
    return *i;
  }
}

/**************************************************************************/

GledViewNS::ClassInfo* GledViewNS::ClassInfo::GetParentCI()
{
  if(!fParentCI && !fParentName.empty()) {
    FID_t fid = GledNS::FindClass(fParentName);
    fParentCI = const_cast<ClassInfo*>(GledViewNS::FindClassInfo(fid));
  }
  return fParentCI;
}

GledViewNS::ClassInfo* GledViewNS::ClassInfo::GetRendererCI()
{
  if(!fRendererCI && !fRendererGlass.empty()) {
    FID_t fid = GledNS::FindClass(fRendererGlass);
    fRendererCI = const_cast<ClassInfo*>(GledViewNS::FindClassInfo(fid));
  }
  return fRendererCI;
}

A_Rnr* GledViewNS::ClassInfo::SpawnRnr(const string& rnr, ZGlass* g)
{
  if(fRendererCI == 0) GetRendererCI();
  //cout <<"GledViewNS::ClassInfo::SpawnRnr rnr="<< rnr <<", lens="<< g->GetName() <<
  //"["<< fRendererCI->fClassName <<"]\n";
  return GledViewNS::SpawnRnr(rnr, g, fRendererCI->fFid.lid, fRendererCI->fFid.cid);
}

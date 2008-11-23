// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GledNS.h"
#include <Glasses/ZGlass.h>
#include <Stones/ZMIR.h>
#include <Ephra/Saturn.h>
#include <Gled/GMutex.h>
#include <Gled/Gled.h>

#include <TMessage.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TSystem.h>
#include <TVirtualMutex.h>

#include <TROOT.h>
#include <TClass.h>
#include <TRealData.h>
#include <TDataMember.h>
#include <TPRegexp.h>


#include <stack>

#include <G__ci.h> // CINT's G__ global functions.
#include <dlfcn.h> // Needed for FindSymbol, CINT's version does not
		   // work until the first library is actually loaded.

/**************************************************************************/

int		G_DEBUG = 0;

struct FD_pair {
  TFile* file; TDirectory* dir;
  FD_pair(TFile* f, TDirectory* d) : file(f), dir(d) {}
};

namespace GledNS {

  TDirectory* GledRoot = 0;

  stack<FD_pair>	FDstack;
  GMutex		FDmutex(GMutex::recursive);

  hLid2pLSI_t	Lid2LSInfo;	// Catalog of libsets by LibSet ID
  hName2Lid_t	Name2Lid;	// Catalog of libsets by name
  hName2Fid_t	Name2Fid;	// Catalog of glasses by name

} // namespace GledNS

// FDmutex will disappear once threads are ok in root

void GledNS::InitFD(TFile* file, TDirectory* dir)
{
  FDstack.push(FD_pair(file, dir));
}

void GledNS::PushFD()
{
  FDmutex.Lock();
  FDstack.push(FD_pair(gFile, gDirectory));
}

void GledNS::PopFD()
{
  gFile = FDstack.top().file;
  if(FDstack.top().dir != 0) {
    FDstack.top().dir->cd();
  } else {
    // dump some error
  }
  FDstack.pop();
  FDmutex.Unlock();
}

/**************************************************************************/

Int_t GledNS::LoadSoSet(const TString& lib_set)
{
  TString libname = FabricateLibName(lib_set);
  Int_t ret = LoadSo(libname);
  if(ret) {
    ISmess(GForm("GledNS::LoadSoSet loading %s as %s returned %d",
		  lib_set.Data(), libname.Data(), ret));
  }
  if(ret < 0) return ret;
  ret = InitSoSet(lib_set);
  if(ret) return ret;
  AssertRenderers();
  return ret;
}

Int_t GledNS::InitSoSet(const TString& lib_set)
{
  { // init
    TString cmd = FabricateInitFoo(lib_set);
    long* p2foo = (long*) FindSymbol(cmd);
    if(!p2foo) {
      ISerr(GForm("GledNS::InitSoSet can't find %s. Safr!", cmd.Data()));
      return 2;
    }
    void (*foo)() = (void(*)())(*p2foo);
    foo();
  }
  { // user_init
    TString cmd = FabricateUserInitFoo(lib_set);
    long* p2foo = (long*) FindSymbol(cmd);
    if(!p2foo) {
      ISmess(GForm("GledNS::InitSoSet no user initialization for %s.",
		   lib_set.Data()));
    } else {
      ISmess(GForm("GledNS::InitSoSet execing user initialization for %s.",
		   lib_set.Data()));
      void (*foo)() = (void(*)())(*p2foo);
      foo();
    }
  }
  return 0;
}

Int_t GledNS::LoadSo(const TString& full_lib_name)
{
  G__Set_RTLD_LAZY();
  Int_t ret = gSystem->Load(full_lib_name.Data());
  if(ret) {
    ISmess(GForm("GledNS::LoadSo loading %s returned %d",
		 full_lib_name.Data(), ret));
  }
  return ret;
}


void* GledNS::FindSymbol(const TString& sym)
{
  // Used to be G__findsym( sym );
  // Didn't work for explicitly linked libs. Guess those are not registered in cint.

  // dlsym for APPLE takes arg WITHOUT the '_'.
  return dlsym(RTLD_DEFAULT, sym.Data());
}

/**************************************************************************/

void GledNS::BootstrapSoSet(LibSetInfo* lsi)
{
  hLid2pLSI_i i = Lid2LSInfo.find(lsi->fLid);
  if(i != Lid2LSInfo.end()) {
    ISwarn(GForm("GledNS::BootstrapSoSet %s(id=%u) already loaded ...",
		 i->second->fName.Data(), lsi->fLid));
    return;
  }
  ISmess(GForm("GledNS::BootstrapSoSet installing %s(id=%u) ...",
	       lsi->fName.Data(), lsi->fLid));
  Lid2LSInfo[lsi->fLid] = lsi;
  Name2Lid[lsi->fName] = lsi->fLid;
  // Init deps as well ... loaded by link-time dependence
  const char** dep = lsi->fDeps;
  while(*dep) {
    if(Name2Lid.find(*dep) == Name2Lid.end()) {
      Int_t ini = InitSoSet(*dep);
      if(ini) return;
    }
    ++dep;
  }
}

/**************************************************************************/

bool GledNS::IsLoaded(const TString& lib_set)
{
  return (Name2Lid.find(lib_set) != Name2Lid.end());
}

bool GledNS::IsLoaded(LID_t lid)
{
  return (Lid2LSInfo.find(lid) != Lid2LSInfo.end());
}

/**************************************************************************/

void GledNS::BootstrapClass(GledNS::ClassInfo* ci)
{
  // !!!! no check done if class already registered
  // As well ... perhaps should separate them by LID
  // will be thinking of that later ...
  LibSetInfo* lsi = FindLibSetInfo(ci->fFid.fLid);
  lsi->Cid2CInfo[ci->fFid.fCid] = ci;
  Name2Fid.insert(pair<TString,FID_t>(ci->fName, ci->fFid));
}

/**************************************************************************/

TString GledNS::FabricateLibName(const TString& libset)
{
  return TString("lib") + libset + ".so";
}

TString GledNS::FabricateInitFoo(const TString& libset)
{
  // Returns name of void* pointing to init_foo

  return libset + "_GLED_init";
}

TString GledNS::FabricateUserInitFoo(const TString& libset)
{
  // Returns name of void* pointing to user_init_foo

  return libset + "_GLED_user_init";
}

/**************************************************************************/

namespace GledNS {
  set<TString>	RnrNames;
}

void GledNS::BootstrapRnrSet(const TString& libset, LID_t lid,
			     const TString& rnr, A_Rnr_Creator_foo rfoo)
{
  static const Exc_t _eh("GledNS::BootstrapRnrSet ");

  LibSetInfo* lsi = FindLibSetInfo(lid);
  if(lsi == 0) {
    ISwarn(_eh + GForm("LibSet %s(lid=%u) not loaded.", libset.Data(), lid));
    return;
  }
  hRnr2RCFoo_i j = lsi->Rnr2RCFoo.find(rnr);
  if(j != lsi->Rnr2RCFoo.end()) {
    ISwarn(_eh + GForm("RnrCreator for rnr=%s, LibSet=%s (lid=%u) already present.",
		       rnr.Data(), libset.Data(), lid));
    return;
  }
  lsi->Rnr2RCFoo[rnr] = rfoo;
}

TString GledNS::FabricateRnrLibName(const TString& libset, const TString& rnr)
{
  return TString("lib") + libset + "_Rnr_" + rnr + ".so";
}

TString GledNS::FabricateRnrInitFoo(const TString& libset, const TString& rnr)
{
  TString foo = libset + "_GLED_init_Rnr_" + rnr;
  return foo;
}


/**************************************************************************/

void GledNS::AssertRenderers()
{
  static const Exc_t _eh("GledNS::AssertRenderers ");

  lpLSI_t ls_list;
  ProduceLibSetInfoList(ls_list);
  for(lpLSI_i lsi=ls_list.begin(); lsi!=ls_list.end(); ++lsi) {
    TString libset = (*lsi)->fName;
    for(set<TString>::iterator rnr=RnrNames.begin(); rnr!=RnrNames.end(); ++rnr) {
      if((*lsi)->Rnr2RCFoo.find(*rnr) == (*lsi)->Rnr2RCFoo.end()) {
	TString cmd = FabricateRnrInitFoo(libset, *rnr);
	long* p2foo = (long*) FindSymbol(cmd);
	if(!p2foo) {
	  TString libname = FabricateRnrLibName(libset, *rnr);
	  int ret = LoadSo(libname);
	  if(ret < 0) {
	    ISerr(_eh + libname + " not existing.");
	    return;
	  }
	  p2foo = (long*) FindSymbol(cmd);
	  if(!p2foo) {
	    ISerr(_eh + cmd + " not existing in " + libname + ".");
	    return;
	  }
	}
	void (*foo)() = (void(*)())(*p2foo);
	foo();
      }
    }
  }
}

void GledNS::AddRenderer(const TString& rnr)
{
  if(RnrNames.find(rnr) == RnrNames.end()) {
    RnrNames.insert(rnr);
    AssertRenderers();
  }
}

A_Rnr* GledNS::SpawnRnr(const TString& rnr, ZGlass* d, FID_t fid)
{
  static const Exc_t _eh("GledNS::SpawnRnr ");

  LibSetInfo* lsi = FindLibSetInfo(fid.fLid);
  if(lsi == 0) {
    ISerr(_eh + GForm("can't demangle lib id=%u.", fid.fLid));
    return 0;
  }
  hRnr2RCFoo_i j = lsi->Rnr2RCFoo.find(rnr);
  if(j == lsi->Rnr2RCFoo.end()) {
    ISerr(_eh + GForm("can't find Rnr Constructor for %s.", rnr.Data()));
    return 0;
  }
  return (j->second)(d, fid.fCid);
}

/**************************************************************************/
/**************************************************************************/

ZGlass* GledNS::ConstructLens(FID_t fid)
{
  LibSetInfo* lsi = FindLibSetInfo(fid.fLid);
  if(lsi == 0) {
    ISerr(GForm("GledNS::ConstructLens lib set %u not found", fid.fLid));
    return 0;
  }
  ZGlass* g = (lsi->fLC_Foo)(fid.fCid);
  if(g == 0) {
    ISerr(GForm("GledNS::ConstructLens default ctor for FID_t(%u,%u) returned 0", fid.fLid, fid.fCid));
    return 0;
  }
  return g;
}

bool GledNS::IsA(ZGlass* glass, FID_t fid)
{
  if(fid.is_null()) return true;
  LibSetInfo* lsi = FindLibSetInfo(fid.fLid);
  if(lsi == 0) return false;
  return (lsi->fISA_Foo)(glass, fid.fCid);
}

/**************************************************************************/

#if ROOT_VERSION_CODE >= ROOT_VERSION(5,0,0)
  #define ROOT_CINT_MUTEX gGlobalMutex
#else
  #define ROOT_CINT_MUTEX gCINTMutex
#endif

TVirtualMutex* GledNS::GetCINTMutex() { return ROOT_CINT_MUTEX; }

void GledNS::LockCINT()   { ROOT_CINT_MUTEX->Lock();   }
void GledNS::UnlockCINT() { ROOT_CINT_MUTEX->UnLock(); }

/**************************************************************************/

void GledNS::StreamLens(TBuffer& b, ZGlass* lens)
{
  // Writes lens, prefixed by Lid/Cid to the buffer.

  assert(b.IsWriting());
  b << lens->VFID();
  R__LOCKGUARD(ROOT_CINT_MUTEX);
  lens->Streamer(b);
}

ZGlass* GledNS::StreamLens(TBuffer& b)
{
  // Reads lid/cid of the glass, instantiates a lens and streams it out.

  assert(b.IsReading());
  FID_t fid;
  b >> fid;
  ZGlass *lens = ConstructLens(fid);
  if(lens) {
    R__LOCKGUARD(ROOT_CINT_MUTEX);
    lens->Streamer(b);
  }
  return lens;
}

void GledNS::WriteLensID(TBuffer& b, ZGlass* lens)
{
  // Writes lens ID to the buffer.

  assert(b.IsWriting());
  b << (lens ? lens->GetSaturnID() : ID_t(0));
}

ID_t GledNS::ReadLensID(TBuffer& b)
{
  assert(b.IsReading());
  ID_t id;
  b >> id;
  return id;
}

ZGlass* GledNS::ReadLensIDAsPtr(TBuffer& b)
{
  assert(b.IsReading());
  ID_t id;
  b >> id;
  char* p = 0; p += id;
  return (ZGlass*) p;
}

/**************************************************************************/
/**************************************************************************/
// Implementations of Finders and Info-class methods
/**************************************************************************/
/**************************************************************************/

GledNS::LibSetInfo* GledNS::FindLibSetInfo(LID_t lid)
{
  hLid2pLSI_i i = Lid2LSInfo.find(lid);
  if(i == GledNS::Lid2LSInfo.end()) {
    ISerr(GForm("GledNS::FindLibSetInfo can't demangle lib id=%u", lid));
    return 0;
  }
  return i->second;
}

GledNS::LibSetInfo* GledNS::FindLibSetInfo(const TString& lib_set)
{
  hName2Lid_i i = Name2Lid.find(lib_set);
  return (i != Name2Lid.end()) ? FindLibSetInfo(i->second) : 0;
}


void GledNS::ProduceLibSetInfoList(lpLSI_t& li_list)
{
  for(hLid2pLSI_i i=Lid2LSInfo.begin(); i!=Lid2LSInfo.end(); ++i) {
    LID_t lid = i->second->fLid;
    lpLSI_i l = li_list.begin();
    while(l != li_list.end() && lid > (*l)->fLid) ++l;
    li_list.insert(l, i->second);
  }
}

GledNS::ClassInfo* GledNS::FindClassInfo(FID_t fid)
{
  if(fid.is_null()) return 0;
  hLid2pLSI_i i = Lid2LSInfo.find(fid.fLid);
  if(i == GledNS::Lid2LSInfo.end()) {
    ISerr(GForm("GledNS::FindClassInfo can't demangle lib id=%u", fid.fLid));
    return 0;
  }
  return i->second->FindClassInfo(fid.fCid);
}

FID_t GledNS::FindClassID(const TString& name)
{
  hName2Fid_i i = Name2Fid.find(name);
  return (i != Name2Fid.end()) ? i->second : FID_t(0,0);
}

GledNS::ClassInfo* GledNS::FindClassInfo(const TString& name)
{
  return FindClassInfo(FindClassID(name));
}

/**************************************************************************/

GledNS::MethodInfo* GledNS::DeduceMethodInfo(ZGlass* alpha, const TString& name)
{
  // 'name' can be FQ, eg. "SomeGlass::Foo".

  TPMERegexp re("::");
  int  ret = re.Split(name);
  if(ret == 2) {
    ClassInfo* ci = FindClassInfo(re[0]);
    if(ci == 0)  return 0;
    return ci->FindMethodInfo(re[1], false);
  }
  else if(ret == 1 && alpha != 0) {
    ClassInfo* ci = alpha->VGlassInfo();
    return ci->FindMethodInfo(re[0], true);
  }
  return 0;
}

GledNS::DataMemberInfo* GledNS::DeduceDataMemberInfo(ZGlass* alpha, const TString& name)
{
  // 'name' can be FQ, eg. "SomeGlass::Foo".

  TPMERegexp re("::");
  int  ret = re.Split(name);
  if(ret == 2) {
    ClassInfo* ci = FindClassInfo(re[0]);
    if(ci == 0)  return 0;
    return ci->FindDataMemberInfo(re[1], false);
  }
  else if(ret == 1 && alpha != 0) {
    ClassInfo* ci = alpha->VGlassInfo();
    return ci->FindDataMemberInfo(re[0], true);
  }
  return 0;
}

/**************************************************************************/
// GledNS::MethodInfo
/**************************************************************************/

ZMIR* GledNS::MethodInfo::MakeMir(ZGlass* a, ZGlass* b, ZGlass* g)
{
  ZMIR* mir = new ZMIR(a, b, g);
  ImprintMir(*mir);
  return mir;
}

void GledNS::MethodInfo::ImprintMir(ZMIR& mir) const
{
  mir.SetLCM_Ids(fClassInfo->fFid.fLid, fClassInfo->fFid.fCid, fMid);
  if(bLocal)       mir.SetRecipient(0);
  if(bDetachedExe) mir.SetDetachedExe(bMultixDetachedExe);
}

void GledNS::MethodInfo::StreamIds(TBuffer& b) const
{
  assert(b.IsWriting());
  b << fClassInfo->fFid.fLid << fClassInfo->fFid.fCid << fMid;
}

/**************************************************************************/
// GledNS::DataMemberInfo
/**************************************************************************/

TString GledNS::DataMemberInfo::CName()
{ return fPrefix + fName; }

TString GledNS::DataMemberInfo::FullName()
{ return fClassInfo->fName + "::" + fName; }

TString GledNS::DataMemberInfo::FullCName()
{ return fClassInfo->fName + "::" + CName(); }

TRealData* GledNS::DataMemberInfo::GetTRealData()
{
  if(fTRealData == 0)
    fTRealData = fClassInfo->GetTClass()->GetRealData(CName().Data());
  return fTRealData;
}

TDataMember* GledNS::DataMemberInfo::GetTDataMember()
{
  return (GetTRealData()) ? fTRealData->GetDataMember() : 0;
}

/**************************************************************************/
// GledNS::ClassInfo
/**************************************************************************/

GledNS::lpDataMemberInfo_t*
GledNS::ClassInfo::ProduceFullDataMemberInfoList()
{
  // Recursive up call towards the base (ZGlass)
  lpDataMemberInfo_t* ret;
  ClassInfo* p = GetParentCI();
  if(p) ret = p->ProduceFullDataMemberInfoList();
  else	ret = new lpDataMemberInfo_t;
  copy(fDataMemberList.begin(), fDataMemberList.end(), back_inserter(*ret));
  return ret;
}

GledNS::lpLinkMemberInfo_t*
GledNS::ClassInfo::ProduceFullLinkMemberInfoList()
{
  // Recursive up call towards the base (ZGlass)
  lpLinkMemberInfo_t* ret;
  ClassInfo* p = GetParentCI();
  if(p) ret = p->ProduceFullLinkMemberInfoList();
  else	ret = new lpLinkMemberInfo_t;
  copy(fLinkMemberList.begin(), fLinkMemberList.end(), back_inserter(*ret));
  return ret;
}

/**************************************************************************/

GledNS::ClassInfo* GledNS::ClassInfo::GetRendererCI()
{
  if(!fRendererCI && !fRendererGlass.IsNull()) {
    TString rnr_glass = fRendererGlass;
    while(true) {
      fRendererCI = GledNS::FindClassInfo(rnr_glass);
      if(fRendererCI == 0)
        break;
      rnr_glass = fRendererCI->fRendererGlass;
      if(fRendererCI->fName == rnr_glass)
	break;
    }
  }
  return fRendererCI;
}

A_Rnr* GledNS::ClassInfo::SpawnRnr(const TString& rnr, ZGlass* g)
{
  if(fRendererCI == 0) GetRendererCI();
  //cout <<"GledNS::ClassInfo::SpawnRnr rnr="<< rnr <<", lens="<< g->GetName() <<
  //  "["<< fRendererCI->fName <<"]\n";
  return GledNS::SpawnRnr(rnr, g, fRendererCI->fFid);
}

/**************************************************************************/

namespace {
  struct infobase_name_eq : public unary_function<GledNS::InfoBase*, bool> {
    TString name;
    infobase_name_eq(const TString& s) : name(s) {}
    bool operator()(const GledNS::InfoBase* ib) {
      return ib->fName == name; }
  };
}

GledNS::MethodInfo*
GledNS::ClassInfo::FindMethodInfo(MID_t mid)
{
  hMid2pMethodInfo_i i = fMethodHash.find(mid);
  return ( i!= fMethodHash.end()) ? i->second : 0;
}

GledNS::MethodInfo*
GledNS::ClassInfo::FindMethodInfo(const TString& func_name, bool recurse, bool throwp)
{
  static const Exc_t _eh("GledNS::ClassInfo::FindMethodInfo ");

  lpMethodInfo_i i = find_if(fMethodList.begin(), fMethodList.end(),
			     infobase_name_eq(func_name));
  if(i != fMethodList.end()) return *i;
  if(recurse) {
    ClassInfo* p = GetParentCI();
    if(p) return p->FindMethodInfo(func_name, recurse);
  }
  if(throwp)
    throw(_eh + GForm("'%s' not found in glass %s (recurse=%d).",
		      func_name.Data(), fName.Data(), recurse));
  return 0;
}

GledNS::DataMemberInfo*
GledNS::ClassInfo::FindDataMemberInfo(const TString& s, bool recurse, bool throwp)
{
  static const Exc_t _eh("GledNS::ClassInfo::FindDataMemberInfo ");

  lpDataMemberInfo_i i = find_if(fDataMemberList.begin(), fDataMemberList.end(),
				infobase_name_eq(s));

  if(i != fDataMemberList.end()) return *i;
  if(recurse) {
    ClassInfo* p = GetParentCI();
    if(p) return p->FindDataMemberInfo(s, recurse);
  }
  if(throwp)
    throw(_eh + GForm("'%s' not found in glass %s (recurse=%d).",
		      s.Data(), fName.Data(), recurse));
  return 0;
}

GledNS::LinkMemberInfo*
GledNS::ClassInfo::FindLinkMemberInfo(const TString& s, bool recurse, bool throwp)
{
  static const Exc_t _eh("GledNS::ClassInfo::FindLinkMemberInfo ");

  lpLinkMemberInfo_i i = find_if(fLinkMemberList.begin(), fLinkMemberList.end(),
				infobase_name_eq(s));

  if(i != fLinkMemberList.end()) return *i;
  if(recurse) {
    ClassInfo* p = GetParentCI();
    if(p) return p->FindLinkMemberInfo(s, recurse);
  }
  if(throwp)
    throw(_eh + GForm("'%s' not found in glass %s (recurse=%d).",
		      s.Data(), fName.Data(), recurse));
  return 0;
}

/**************************************************************************/

GledNS::LibSetInfo* GledNS::ClassInfo::GetLibSetInfo()
{
  if(!fLibSetInfo) fLibSetInfo = FindLibSetInfo(fFid.fLid);
  return fLibSetInfo;
}

GledNS::ClassInfo* GledNS::ClassInfo::GetParentCI()
{
  if(!fParentCI && !fParentName.IsNull()) {
    FID_t fid = GledNS::FindClassID(fParentName);
    fParentCI = const_cast<ClassInfo*>(FindClassInfo(fid));
  }
  return fParentCI;
}

/**************************************************************************/

TClass* GledNS::ClassInfo::GetTClass()
{
  if(fTClass == 0)
    fTClass = gROOT->GetClass(fName.Data(), true);
  return fTClass;
}

/**************************************************************************/
// LibSetInfo
/**************************************************************************/

GledNS::ClassInfo* GledNS::LibSetInfo::FindClassInfo(CID_t cid)
{
  hCid2pCI_i i = Cid2CInfo.find(cid);
  if(i == Cid2CInfo.end()) {
    ISerr(GForm("GledNS::LibSetInfo::FindClassInfo can't demangle class cid=%u", cid));
    return 0;
  }
  return i->second;
}

GledNS::ClassInfo* GledNS::LibSetInfo::FirstClassInfo()
{
  hCid2pCI_i i = Cid2CInfo.begin();
  if(i == Cid2CInfo.end()) {
    ISerr("GledNS::LibSetInfo::FirstClassInfo no classes found");
    return 0;
  }
  return i->second;
}

/**************************************************************************/
/**************************************************************************/
// Value-type peek and MIR-poke
/**************************************************************************/
/**************************************************************************/

// Type snatched from TDataType.h

Double_t GledNS::peek_value(void* addr, Int_t type)
{
  switch(type) {
  case  2: return *((Short_t*)addr);
  case  3: return *((Int_t*)addr);
  case  4: return *((Long_t*)addr);
  case  5: return *((Float_t*)addr);
  case  6: return *((Int_t*)addr);
  case  8: return *((Double_t*)addr);
  case  9: return *((Double32_t*)addr);
  case 11: return *((UChar_t*)addr);
  case 12: return *((UShort_t*)addr);
  case 13: return *((UInt_t*)addr);
  case 14: return *((ULong_t*)addr);
  case 15: return *((UInt_t*)addr);
  case 16: return *((Long64_t*)addr);
  case 17: return *((ULong64_t*)addr);
  case 18: return *((Bool_t*)addr);
  default: return 0;
  };
}

void GledNS::stream_value(TBuffer& b, Int_t type, Double_t value)
{
  assert(b.IsWriting());
  switch(type) {
  case  2: b << (Short_t)value;    break;
  case  3: b << (Int_t)value;      break;
  case  4: b << (Long_t)value;     break;
  case  5: b << (Float_t)value;    break;
  case  6: b << (Int_t)value;      break;
  case  8: b << (Double_t)value;   break;
  case  9: b << (Double32_t)value; break;
  case 11: b << (UChar_t)value;    break;
  case 12: b << (UShort_t)value;   break;
  case 13: b << (UInt_t)value;     break;
  case 14: b << (ULong_t)value;    break;
  case 15: b << (UInt_t)value;     break;
  case 16: b << (Long64_t)value;   break;
  case 17: b << (ULong64_t)value;  break;
  case 18: b << (Bool_t)value;     break;
  default: b << Int_t(0);          break;
  };
}

/**************************************************************************/
/**************************************************************************/
// Simple TString parser
/**************************************************************************/
/**************************************************************************/

int GledNS::split_string(const TString& s, Ssiz_t start, Ssiz_t end,
			 lStr_t& l, char c)
{
  int cnt=0;
  TString g;
  for(Ssiz_t i=start; i!=end; ++i) {
    if(c==0) {
      if(isspace(s(i)) && g.Length()>0) {
	++cnt; l.push_back(g); g = (char)0; continue;
      }
      if(isspace(s(i))) continue;
    }
    if(s(i)==c) {
      ++cnt; l.push_back(g); g = (char)0; continue;
    }
    g += s(i);
  }
  if(g.Length()>0) { ++cnt; l.push_back(g); }
  return cnt;
}

int GledNS::split_string(const TString& s, lStr_t& l, char c)
{
  // Splits TString on character c. If c==0 splits on whitespace.
  return split_string(s, 0, s.Length(), l, c);
}

int GledNS::split_string(const TString& s, lStr_t& l, const TString& ptr)
{
  // Splits TString on whole contents of ptr.

  int cnt = 0;
  Ssiz_t       i      = 0;
  const Ssiz_t end    = s.Length();
  const Ssiz_t ptrlen = ptr.Length();

  while(i < end) {
    Ssiz_t j = s.Index(ptr, i);
    if(j == kNPOS) j = end;
    if(j > i) {
      ++cnt;
      l.push_back(s(i, j-i));
    }
    i = j + ptrlen;
  }
  return cnt;
}

void GledNS::deparen_string(const TString& in, TString& n, TString& a,
			    const TString& ops, bool no_parens_ok)
  throw (Exc_t)
{
  // expects back parens to be the last char ... could as well grep it
  static const Exc_t _eh("GledNS::deparen_string ");

  Ssiz_t op_pos = in.First(ops);
  if(op_pos == kNPOS) {
    if(no_parens_ok) {
      if(in.IsNull()) throw _eh + "missing name.";
      n = in;
      return;
    } else {
      throw _eh + "no open paren.";
    }
  }
  int cp_pos = in.Length()-1;
  char o = in(op_pos);
  char c = in(cp_pos);
  if((o=='(' && c!=')') || (o=='[' && c!=']') || (o=='{' && c!='}')) {
    throw _eh + "no close paren.";
  }
  n = in;
  n.Remove(op_pos, cp_pos-op_pos+1);
  a = in(op_pos+1, cp_pos-op_pos-1);
  if(n.Length()==0) throw _eh + "missing name";
  if(a.Length()==0) throw _eh + "no args for " + n + ".";
}

/**************************************************************************/

TString GledNS::join_strings(const TString& sep, lStr_t& list)
{
  if(list.empty()) return "";
  lStr_i i = list.begin();
  TString ret = *i;
  while(++i != list.end()) ret += sep + *i;
  return ret;
}

/**************************************************************************/

void GledNS::remove_whitespace(TString& s)
{
  TString g;
  Ssiz_t end = s.Length();
  for(Ssiz_t i=0; i!=end; ++i) {
    if(!isspace(s(i))) g += s(i);
  }
  s = g;
}

Ssiz_t GledNS::find_first_of(const TString& s, const char* accept, Ssiz_t i)
{
  if(i >= s.Length()) return kNPOS;
  const char *f = strpbrk(s.Data()+i, accept);
  return f ? f - s.Data() : kNPOS;
}

/**************************************************************************/
// Argument/type-name parsing foos
/**************************************************************************/

void GledNS::split_argument(const TString& arg,
			    TString& type, TString& name, TString& def)
{
  Ssiz_t ei = arg.First('=');
  if(ei != kNPOS) {
    Ssiz_t tei = ei+1; while(isspace(arg(tei))) ++tei;
    def = arg(tei, arg.Length()-tei);
  } else {
    ei = arg.Length();
  }
  --ei;
  while(isspace(arg(ei))) --ei;
  Ssiz_t ti = ei;
  while(isalnum(arg(ti)) || arg(ti) == '_') --ti;
  name = arg(ti+1, ei-ti);
  while(isspace(arg(ti))) --ti;
  type = arg(0, ti+1);
}

void GledNS::unrefptrconst_type(TString& type)
{
  Ssiz_t i;
  while((i = type.First("*&")) != kNPOS) {
    type.Replace(i, 1, " ");
  }
  type.Prepend(" ");
  while((i = type.Index(" const ")) != kNPOS) {
    type.Remove(i+1, 5);
  }
  remove_whitespace(type);
}

/**************************************************************************/
/**************************************************************************/

int GledNS::tokenize_url(const TString& url, list<url_token>& l)
{
  Ssiz_t i = 0;
  const Ssiz_t end = url.Length();
  url_token::type_e type      = url_token::list_sel;
  url_token::type_e next_type = url_token::null;
  TString part;
  int count = 0;

  while(i < end) {
    Ssiz_t j = find_first_of(url, "/-\\", i);
    if(j == kNPOS) j = end;
    if(j > i) {
      part += url(i, j-i);
      i = j;
    }
    bool terminal = false;
    char c = url(i); // Relies on url(end) == 0

    if(c == '\\') {
      if(++i < end) part += url[i];
      if(++i < end) continue;

      terminal = true;
      next_type = url_token::null;
    } else {
      // is a terminal?
      switch (c) {
      case 0:
	terminal = true;
	next_type = url_token::null;
	break;
      case '/':
	terminal = true;
	next_type = url_token::list_sel;
	++i;
	break;
      case '-':
	if(i+1 < end && url[i+1] == '>') {
	  terminal = true;
	  next_type = url_token::link_sel;
	  i += 2;
	} else {
	  part += "-";
	  if(++i == end) terminal = true;
	}
	break;
      } // switch
    }
    if(terminal) {
      if(part.Length() > 0) {
	// cout <<"tokenize_url making token " << part <<","<< (int)type <<endl;
	l.push_back(url_token(part, type));
	++count;
	part = "";
      }
      type = next_type;
    }
  }
  return count;
}


/**************************************************************************/
/**************************************************************************/
// Exception stuffe
/**************************************************************************/
/**************************************************************************/

//______________________________________________________________________________
//
// Exception class thrown by Gled classes and macros.

ClassImp(Exc_t);

Exc_t::Exc_t(const std::string& s) : TString(s.c_str())
{
   // Constructor.
}

Exc_t operator+(const Exc_t &s1, const std::string &s2)
{ Exc_t r(s1); r += s2; return r; }

Exc_t operator+(const Exc_t &s1, const TString &s2)
{ Exc_t r(s1); r += s2; return r; }

Exc_t operator+(const Exc_t &s1,  const char *s2)
{ Exc_t r(s1); r += s2; return r; }


/**************************************************************************/
/**************************************************************************/
// Circular formatting buffer with mutex
/**************************************************************************/
/**************************************************************************/

namespace {
  GMutex	form_mutex;
  const int	form_len = 1024 * 16;
  char		form_buffer[form_len];
  char*		form_pos = form_buffer;
  char*		form_end = form_buffer + form_len;
  const int	max_length = 1024;
}

const char* GForm(const char* fmt, ...)
{
  form_mutex.Lock();

  if(form_end - form_pos < max_length) form_pos = form_buffer;

  va_list ap;
  va_start(ap,fmt);
  int n = vsnprintf(form_pos, max_length, fmt, ap);
  va_end(ap);

  if(n >= max_length) {
    n = max_length;
    size_t l = strlen(fmt);
    if(fmt[l-1]==10) form_pos[n-2] = 10;
  } else {
    ++n;
  }
  char* ret = form_pos;
  form_pos += n;

  form_mutex.Unlock();
  return ret;
}

/**************************************************************************/
/**************************************************************************/
// TString .vs. std::TString.
/**************************************************************************/
/**************************************************************************/

TBuffer& operator<<(TBuffer& b, const string& s)
{
  Int_t   nbig;
  UChar_t nwh;
  nbig = s.length();
  if (nbig > 254) {
    nwh = 255;
    b << nwh;
    b << nbig;
  } else {
    nwh = UChar_t(nbig);
    b << nwh;
  }
  b.WriteFastArray(s.data(), nbig);
  return b;
}

TBuffer& operator>>(TBuffer& b, string& s)
{
   Int_t   nbig;
   UChar_t nwh;
   b >> nwh;
   if (nwh == 255)
     b >> nbig;
   else
     nbig = nwh;
   s.resize(nbig);
   b.ReadFastArray(const_cast<char*>(s.data()), nbig);
   return b;
}

// Relies on TString::Data() fast & null-terminated.

bool operator==(const TString& t, const string& s)
{ return (s == t.Data()); }

bool operator==(const string&  s, const TString& t)
{ return (s == t.Data()); }

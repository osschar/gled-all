// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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

#include <stack>

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

//template<class C>
// void GledNS::Beam(TBuffer* b, C c) { *b << c; }
// void GledNS::Beam(TBuffer* b, Saturn s) {}
// template<class C>
// void GledNS::Qeam(TBuffer* b, C& c) { *b >> c; }

// template<class C>
// void GledNS::Beam(TBuffer* b, C* c) { *b << *c; }
// void GledNS::Beam(TBuffer* b, Text_t* c) { b->WriteArray(c, strlen(c+1)); }
// template<class C>
// void GledNS::Qeamp(TBuffer* b, C*& c) { c=new C; *b >> *c; }

// void GledNS::Qeamp(TBuffer* b, Text_t*& c) { b->ReadArray(c); }

// void GledNS::Beam(TBuffer* b, ZNode* c) { *b << c->GetSaturnID(); }
// void GledNS::Qeam(TBuffer* b, ZNode* c, ZNode* caller) {
//   if(!caller) { c=0; return; }
//   ID_t x; *b >> x; c = caller->GetSaturn()->DemangleID(x);
// }

/**************************************************************************/
#include <G__ci.h>

Int_t GledNS::LoadSoSet(const string& lib_set)
{
  string libname = FabricateLibName(lib_set);
  Int_t ret = LoadSo(libname);
  if(ret) {
    ISmess(GForm( "GledNS::LoadSoSet loading %s as %s returned %d",
		  lib_set.c_str(), libname.c_str(), ret));
  }
  if(ret < 0) return ret;
  return InitSoSet(lib_set);
}

Int_t GledNS::InitSoSet(const string& lib_set)
{
  { // init
    string cmd = FabricateInitFoo(lib_set);
    long* p2foo = (long*) G__findsym( cmd.c_str() );
    if(!p2foo) {
      ISerr(GForm("GledNS::InitSoSet can't find %s. Safr!", cmd.c_str()));
      return 2;
    }
    void (*foo)() = (void(*)())(*p2foo);
    foo();
  }
  { // user_init
    string cmd = FabricateUserInitFoo(lib_set);
    long* p2foo = (long*) G__findsym( cmd.c_str() );
    if(!p2foo) {
      ISmess(GForm("GledNS::InitSoSet no user initialization for %s.",
		   lib_set.c_str()));
    } else {
      ISmess(GForm("GledNS::InitSoSet execing user initialization for %s.",
		   lib_set.c_str()));
      void (*foo)() = (void(*)())(*p2foo);
      foo();
    }
  }
  return 0;
}

Int_t GledNS::LoadSo(const string& full_lib_name)
{
  G__Set_RTLD_LAZY();
  Int_t ret = gSystem->Load(full_lib_name.c_str());
  if(ret) {
    ISmess(GForm("GledNS::LoadSo loading %s returned %d",
		 full_lib_name.c_str(), ret));
  }
  return ret;
}

void GledNS::BootstrapSoSet(LibSetInfo* lsi)
{
  hLid2pLSI_i i = Lid2LSInfo.find(lsi->fLid);
  if(i != Lid2LSInfo.end()) {
    ISwarn(GForm("GledNS::BootstrapSoSet %s(id=%u) already loaded ...",
		 i->second->fName.c_str(), lsi->fLid));
    return;
  }
  ISmess(GForm("GledNS::BootstrapSoSet installing %s(id=%u) ...",
	       lsi->fName.c_str(), lsi->fLid));
  Lid2LSInfo[lsi->fLid] = lsi;
  Name2Lid[lsi->fName] = lsi->fLid;
  // Init deps as well ... loaded by link-time dependence
  char** dep = lsi->fDeps;
  while(*dep) {
    if(Name2Lid.find(*dep) == Name2Lid.end()) {
      Int_t ini = InitSoSet(*dep);
      if(ini) return;
    }
    ++dep;
  }
}

/**************************************************************************/

bool GledNS::IsLoaded(const string& lib_set)
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
  LibSetInfo* lsi = FindLibSetInfo(ci->fFid.lid);
  lsi->Cid2CInfo[ci->fFid.cid] = ci;
  Name2Fid.insert(pair<string,FID_t>(ci->fName, ci->fFid));
}

/**************************************************************************/

string GledNS::FabricateLibName(const string& libset)
{
  return string("lib") + libset + ".so";
}

string GledNS::FabricateInitFoo(const string& libset)
{
  // Returns name of void* pointing to init_foo

  string foo = libset + "_GLED_init";
  return foo;
}

string GledNS::FabricateUserInitFoo(const string& libset)
{
  // Returns name of void* pointing to init_foo

  string foo = libset + "_GLED_user_init";
  return foo;
}

/**************************************************************************/

ZGlass* GledNS::ConstructLens(LID_t lid, CID_t cid)
{
  LibSetInfo* lsi = FindLibSetInfo(lid);
  if(lsi == 0) {
    ISerr(GForm("GledNS::ConstructLens lib set %u not found", lid));
    return 0;
  }
  ZGlass* g = (lsi->fLC_Foo)(cid);
  if(g == 0) {
    ISerr(GForm("GledNS::ConstructLens default ctor for lid,cid:%u,%u returned 0", lid, cid));
    return 0;
  }
  return g;
}

bool GledNS::IsA(ZGlass* glass, FID_t fid)
{
  LibSetInfo* lsi = FindLibSetInfo(fid.lid);
  if(lsi == 0) return false;
  return (lsi->fISA_Foo)(glass, fid.cid);
}

/**************************************************************************/

void GledNS::StreamLens(TBuffer& b, ZGlass* lens)
{
  // Writes glass, prefixed by Lid/Cid to the buffer.

  assert(b.IsWriting());
  b << lens->ZibID() << lens->ZlassID();
  R__LOCKGUARD(gCINTMutex);
  lens->Streamer(b);
}

ZGlass* GledNS::StreamLens(TBuffer& b)
{
  // Reads lid/cid of the glass, instantiates it and streams it out.

  assert(b.IsReading());
  LID_t lid; CID_t cid;
  b >> lid >> cid;
  ZGlass *lens = ConstructLens(lid, cid);
  if(lens) {
    R__LOCKGUARD(gCINTMutex);
    lens->Streamer(b);
  }
  return lens;
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

GledNS::LibSetInfo* GledNS::FindLibSetInfo(const string& lib_set)
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
  hLid2pLSI_i i = Lid2LSInfo.find(fid.lid);
  if(i == GledNS::Lid2LSInfo.end()) {
    ISerr(GForm("GledNS::FindClassInfo can't demangle lib id=%u", fid.lid));
    return 0;
  }
  return i->second->FindClassInfo(fid.cid);
}

FID_t GledNS::FindClassID(const string& name)
{
  hName2Fid_i i = Name2Fid.find(name);
  return (i != Name2Fid.end()) ? i->second : FID_t(0,0);
}

GledNS::ClassInfo* GledNS::FindClassInfo(const string& name)
{
  return FindClassInfo(FindClassID(name));
}

/**************************************************************************/
// GledNS::MethodInfo
/**************************************************************************/

void GledNS::MethodInfo::ImprintMir(ZMIR& mir) const
{
  mir.SetLCM_Ids(fClassInfo->fFid.lid, fClassInfo->fFid.cid, fMid);
}

void GledNS::MethodInfo::BeamofyIfLocal(ZMIR& mir, SaturnInfo* sat) const
{
  if(bLocal) mir.SetRecipient(sat);
}

void GledNS::MethodInfo::StreamIds(TBuffer& b) const
{
  assert(b.IsWriting());
  b << fClassInfo->fFid.lid << fClassInfo->fFid.cid << fMid;
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

namespace {
  struct infobase_name_eq : public unary_function<GledNS::InfoBase*, bool> {
    string name;
    infobase_name_eq(const string& s) : name(s) {}
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
GledNS::ClassInfo::FindMethodInfo(const string& func_name, bool recurse)
{
  lpMethodInfo_i i = find_if(fMethodList.begin(), fMethodList.end(),
			     infobase_name_eq(func_name));
  if(i != fMethodList.end()) return *i;
  if(recurse) {
    ClassInfo* p = GetParentCI();
    if(p) return p->FindMethodInfo(func_name, recurse);
  }
  return 0;
}

GledNS::DataMemberInfo*
GledNS::ClassInfo::FindDataMemberInfo(const string& s, bool recurse)
{
  lpDataMemberInfo_i i = find_if(fDataMemberList.begin(), fDataMemberList.end(),
				infobase_name_eq(s));
  
  if(i != fDataMemberList.end()) return *i;
  if(recurse) {
    ClassInfo* p = GetParentCI();
    if(p) return p->FindDataMemberInfo(s, recurse);
  }
  return 0;
}

GledNS::LinkMemberInfo*
GledNS::ClassInfo::FindLinkMemberInfo(const string& s, bool recurse)
{
  lpLinkMemberInfo_i i = find_if(fLinkMemberList.begin(), fLinkMemberList.end(),
				infobase_name_eq(s));
  
  if(i != fLinkMemberList.end()) return *i;
  if(recurse) {
    ClassInfo* p = GetParentCI();
    if(p) return p->FindLinkMemberInfo(s, recurse);
  }
  return 0;
}

/**************************************************************************/

GledNS::ClassInfo* GledNS::ClassInfo::GetParentCI()
{
  if(!fParentCI && !fParentName.empty()) {
    FID_t fid = GledNS::FindClassID(fParentName);
    fParentCI = const_cast<ClassInfo*>(FindClassInfo(fid));
  }
  return fParentCI;
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
// Simple string parser
/**************************************************************************/
/**************************************************************************/

int GledNS::split_string(Str_ci start, Str_ci end, lStr_t& l, char c)
{
  int cnt=0;
  string g;
  for(Str_ci i=start; i!=end; ++i) {
    if(c==0 && isspace(*i) && g.size()>0) {
      ++cnt; l.push_back(g); g.erase(); continue;
    }
    if(isspace(*i)) continue;
    if(*i==c) {
      ++cnt; l.push_back(g); g.erase(); continue;
    }
    g += *i;
  }
  if(g.size()>0) { ++cnt; l.push_back(g); }
  return cnt;
}

int GledNS::split_string(const string& s, lStr_t& l, char c)
{
  // Splits string on character c. If c==0 splits on whitespace.
  return split_string(s.begin(), s.end(), l, c);
}

void GledNS::deparen_string(const string& in, string& n, string& a,
			    const string& ops, bool no_parens_ok)
  throw (string)
{
  // expects back parens to be the last char ... could as well grep it
  unsigned int op_pos = in.find_first_of(ops);
  if(op_pos==string::npos) {
    if(no_parens_ok) {
      if(in.size()==0) throw string("missing name");
      n = in;
      return;
    } else {
      throw string("no open paren");
    }
  }
  int cp_pos = in.size()-1;
  char o = in[op_pos];
  char c = in[in.size()-1];
  if(o=='('&&c!=')' || o=='['&&c!=']' || o=='{'&&c!='}') {
    throw string("no close paren");
  }
  n = in;
  n.replace(op_pos, cp_pos-op_pos+1, "");
  a = in.substr(op_pos+1, cp_pos-op_pos-1);
  if(n.size()==0) throw string("missing name");
  if(a.size()==0) throw string("no args for " + n);
}

/**************************************************************************/
/**************************************************************************/

int GledNS::tokenize_url(const string& url, list<url_token>& l)
{
  string::size_type i = 0;
  const string::size_type end = url.length();
  url_token::type_e type = url_token::list_sel;
  url_token::type_e next_type;
  string part;
  int count = 0;

  while(i < end) {
    string::size_type j = url.find_first_of("/-\\", i);
    if(j == string::npos) j = end;
    if(j > i) {
      part += url.substr(i, j-i);
      i = j;
    }
    bool terminal = false;
    char c = url[i];

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
	if(url[i+1] =='>') {
	  terminal = true;
	  next_type = url_token::link_sel;
	  i += 2;
	}
	break;
      } // switch
    }
    if(terminal) {
      if(part.length() > 0) {
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
    int l = strlen(fmt);
    if(fmt[l-1]==10) form_pos[n-2] = 10;
  } else {
    ++n;
  }
  char* ret = form_pos;
  form_pos += n;

  form_mutex.Unlock();
  return ret;
}

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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

  hLid2pLSI_t	Lid2pLSI;
  hName2Lid_t	Name2Lid;
  hName2Fid_t	Name2Fid;

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
  hLid2pLSI_i i = Lid2pLSI.find(lsi->fLid);
  if(i != Lid2pLSI.end()) {
    ISwarn(GForm("GledNS::BootstrapSoSet %s(id=%u) already loaded ...",
		 i->second->fName.c_str(), lsi->fLid));
    return;
  }
  ISmess(GForm("GledNS::BootstrapSoSet installing %s(id=%u) ...",
	       lsi->fName.c_str(), lsi->fLid));
  Lid2pLSI[lsi->fLid] = lsi;
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
  return (Lid2pLSI.find(lid) != Lid2pLSI.end());
}

GledNS::LibSetInfo* GledNS::FindLSI(LID_t lid)
{
  hLid2pLSI_i i = Lid2pLSI.find(lid);

  if(i == Lid2pLSI.end()) {
    int lr = Gled::theOne->LoadLibSet(lid);
    if(lr)
      return 0;
    i = Lid2pLSI.find(lid);
  }

  return (i != Lid2pLSI.end()) ? i->second : 0;
}

/**************************************************************************/

void GledNS::BootstrapClass(const string& name, LID_t l, CID_t c)
{
  // !!!! no check done if class already registered
  // As well ... perhaps should separate them by LID
  // will be thinking of that later ...
  Name2Fid.insert(pair<string,FID_t>(name,FID_t(l, c)));
}

FID_t GledNS::FindClass(const string& name)
{
  hName2Fid_i i = Name2Fid.find(name);
  return (i != Name2Fid.end()) ? i->second : FID_t(0,0);
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

ZGlass* GledNS::ConstructGlass(LID_t lid, CID_t cid)
{
  LibSetInfo* lsi = FindLSI(lid);
  if(lsi == 0) {
    ISerr(GForm("GledNS::ConstructNode lib set %u not found", lid));
    return 0;
  }
  ZGlass* g = (lsi->fDCFoo)(cid);
  if(g == 0) {
    ISerr(GForm("GledNS::ConstructNode default ctor for lid,cid:%u,%u returned 0", lid, cid));
    return 0;
  }
  return g;
}

ZGlass* GledNS::ConstructGlass(Saturn* s, TBuffer* b)
{
  LID_t lid; *b >> lid;
  LibSetInfo* lsi = FindLSI(lid);
  if(lsi == 0) return 0;
  return (lsi->fECFoo)(s, b);
}

bool GledNS::IsA(ZGlass* glass, FID_t fid)
{
  LibSetInfo* lsi = FindLSI(fid.lid);
  if(lsi == 0) return false;
  return (lsi->fISAFoo)(glass, fid.cid);
}

/**************************************************************************/

void GledNS::StreamGlass(TBuffer& b, ZGlass* glass)
{
  // Writes glass, prefixed by Lid/Cid to the buffer.

  assert(b.IsWriting());
  b << glass->ZibID() << glass->ZlassID();
  glass->Streamer(b);
}

ZGlass* GledNS::StreamGlass(TBuffer& b)
{
  // Reads lid/cid of the glass, instantiates it and streams it out.

  assert(b.IsReading());
  LID_t lid; CID_t cid;
  b >> lid >> cid;
  ZGlass *g = GledNS::ConstructGlass(lid, cid);
  if(g) g->Streamer(b);
  return g;
}

/**************************************************************************/
// Circular formatting buffer with mutex
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

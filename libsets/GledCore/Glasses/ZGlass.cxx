// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// ZGlass
//
// Base class of Gled enabled classes.
// Provides infrastructure for integration with the Gled system.
//
// mGlassBits: collection of flags that allow (memory optimised)
// handling of lenses.

#include "ZGlass.h"
#include "ZGlass.c7"

#include <Glasses/AList.h>
#include <Glasses/ZQueen.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZMirFilter.h>
#include <Stones/ZComet.h>
#include <Gled/GThread.h>

/**************************************************************************/

namespace ZGlassBits {
  UShort_t kFixedName = 0x1;
  UShort_t kDying     = 0x2;
}

/**************************************************************************/

ClassImp(ZGlass);

/**************************************************************************/

void ZGlass::_init()
{
  mSaturn = 0; mQueen = 0;
  mGlassBits = 0; mSaturnID=0;
  mGuard = 0;
  bMIRActive = true; bAcceptRefs = true;
  mRefCount = mMoonRefCount = mSunRefCount = mFireRefCount = mEyeRefCount = 0;
  mTimeStamp = mStampReqTring = 0;
  pspRayAbsorber  = 0;
  pspNameChangeCB = 0;

  bUseDispList  = false;
  bUseNameStack = true;
}

/**************************************************************************/

void ZGlass::set_link_or_die(ZGlass*& link, ZGlass* new_val, FID_t fid)
{
  if(link == new_val) return;
  if(link) link->DecRefCount(this);
  if(new_val) {
    try { new_val->IncRefCount(this); }
    catch(...) {
      if(link) { link = 0; StampLink(fid); }
      throw;
    }
  }
  link = new_val;
  StampLink(fid);
}

/**************************************************************************/

void ZGlass::reference_all()   { reference_links();   }
void ZGlass::unreference_all() { unreference_links(); }

void ZGlass::reference_links() {
  lppZGlass_t link_refs;
  CopyLinkRefs(link_refs);
  for(lppZGlass_i i=link_refs.begin(); i!=link_refs.end(); ++i) {
    if(**i) {
      try {
	(**i)->IncRefCount(this);
      }
      catch(Exc_t& exc) {
	**i = 0;
      }
    }
  }
}

void ZGlass::unreference_links() {
  lppZGlass_t link_refs;
  CopyLinkRefs(link_refs);
  for(lppZGlass_i i=link_refs.begin(); i!=link_refs.end(); ++i) {
    if(**i) {
      (**i)->DecRefCount(this);
    }
  }
}

/**************************************************************************/

Int_t ZGlass::remove_references_to(ZGlass* lens)
{
  // Removes link references to *lens*.
  // Virtual ... lists overrirde it but also call the link version
  // for unreferencing of links.
  // Emits Rays, but does NOT call DecRefCount: this is done by wrapper
  // RemoveReferencesTo() (and sometimes by ZQueen during lens delition).

  lppZGlass_t link_refs;
  CopyLinkRefs(link_refs);
  Int_t n = 0;
  for(lppZGlass_i i=link_refs.begin(); i!=link_refs.end(); ++i) {
    if(**i == lens) {
      **i = 0;
      ++n;
    }
  }
  if(n) StampLink();
  return n;
}

/**************************************************************************/
// Public methods
/**************************************************************************/

Int_t ZGlass::RebuildAllRefs(An_ID_Demangler* idd)
{
  return RebuildLinkRefs(idd);
}

/**************************************************************************/

TString ZGlass::Identify() const
{
  return TString(GForm("'%s'[%d]", mName.Data(), mSaturnID));
}

/**************************************************************************/

void ZGlass::WriteLock()
{
  if(mQueen) mQueen->SubjectWriteLock();
  ReadLock();
}

void ZGlass::WriteUnlock()
{
  ReadUnlock();
  if(mQueen) mQueen->SubjectWriteUnlock();
}


/**************************************************************************/

ZMIR* ZGlass::get_MIR()
{
  return GThread::MIR();
}

ZMIR* ZGlass::assert_MIR_presence(const Exc_t& header, int what)
{
  ZMIR* mir = GThread::MIR();
  if (mir == 0) {
    throw(header + "must be called via a MIR.");
  }
  if ((what & MC_IsFlare) && mir->HasRecipient()) {
    throw(header + "must be called via a flared MIR.");
  }
  if ((what & MC_IsBeam) && !mir->HasRecipient()) {
    throw(header + "must be called via a beamed MIR.");
  }
  if((what & MC_HasResultReq) && !mir->HasResultReq()) {
    throw(header + "must be called with a result request set.");
  }
  return mir;
}

ZMIR* ZGlass::suggest_MIR_presence(const Exc_t& header, int what)
{
  ZMIR* mir = GThread::MIR();
  if (mir == 0) {
    ISwarn(header + "should be called via a MIR.");
    return 0;
  }
  if ((what & MC_IsFlare) && mir->HasRecipient()) {
    ISwarn(header + "should be called via a flared MIR.");
  }
  if ((what & MC_IsBeam) && !mir->HasRecipient()) {
    ISwarn(header + "should be called via a beamed MIR.");
  }
  if ((what & MC_HasResultReq) && !mir->HasResultReq()) {
    ISwarn(header + "should be called with a result request set.");
  }
  return mir;
}

void ZGlass::warn_caller(const TString& warning)
{
  ZMIR* mir = GThread::MIR();
  if (mir && mir->fCaller) {
    auto_ptr<ZMIR> wrn( mir->fCaller->S_Warning(warning.Data()) );
    wrn->SetRecipient(mir->fCaller->HostingSaturn());
    mSaturn->ShootMIR(wrn);
  } else {
    ISwarn(warning);
  }
}

/**************************************************************************/

void ZGlass::SetName(const Text_t* n)
{
  static const Exc_t _eh("ZGlass::SetName ");

  if(mGlassBits & ZGlassBits::kFixedName) {
    Stamp(FID());
    throw(_eh + "lens has FixedName bit set.");
  }
  TString name(n);
  WriteLock();
  if(pspNameChangeCB != 0) {
    for(set<NameChangeCB*>::iterator i=pspNameChangeCB->begin();
	i!=pspNameChangeCB->end(); ++i)
      {
	(*i)->name_change_cb(this, name);
      }
  }
  mName = name.Data();
  Stamp(FID());
  WriteUnlock();
  if(mQueen == this) {
    ZMIR* mir = get_MIR();
    if(mir && ! mir->HasRecipient()) mQueen->BasicQueenChange(*mir);
  }
}

void ZGlass::SetTitle(const Text_t* t)
{
  WriteLock();
  mTitle = t;
  Stamp(FID());
  WriteUnlock();
  if(mQueen == this) {
    ZMIR* mir = get_MIR();
    if(mir && ! mir->HasRecipient()) mQueen->BasicQueenChange(*mir);
  }
}

/**************************************************************************/

ZMirFilter* ZGlass::GetGuard()
{
  return mGuard.get();
}

void ZGlass::SetGuard(ZMirFilter* guard)
{
  // Might want to read-lock if LOCK_SET_METHS is true in project7.
  set_link_or_die(mGuard.ref_link(), guard, FID());
}

/**************************************************************************/

ZGlass::~ZGlass()
{
  delete pspNameChangeCB;
}

/**************************************************************************/

void ZGlass::UpdateGlassView()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_change, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

void ZGlass::UpdateAllViews()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_change, mTimeStamp, FID_t(0,0)));
    mQueen->EmitRay(ray);
  }
}

void ZGlass::ReTriangulate()
{
  mStampReqTring = Stamp();
}

void ZGlass::RebuildRnrScheme()
{
  Stamp(FID_t(0,0), 0x1);
}

/**************************************************************************/

bool ZGlass::IsSunSpace()
{
  // Returns true if this lens is in local Saturn's sun-space.

  ZKing* sk = mSaturn->GetKing();
  return (mSaturnID >= sk->GetMinID() && mSaturnID <= sk->GetMaxID());
}

bool ZGlass::IsSunOrFireSpace()
{
  // Returns true if this lens is in local Saturn's sun or fire-space.

  return (mSaturnID >= mSaturn->GetKing()->GetMinID());
}

/**************************************************************************/

Short_t ZGlass::IncRefCount(ZGlass* from)
{
  // Called to notify *this* that *from* desires to reference it.
  // Throws an exception if the reference is not allowed by *this*
  // lens or its queen.

  static const Exc_t _eh("ZGlass::IncRefCount ");

  if(mQueen && from->mQueen) {
    mQueen->SubjectRefCntLock();
    // !!! Here ask queen if it's OK
    if(!bAcceptRefs) {
      mQueen->SubjectRefCntUnlock();
      throw(_eh + "lens not accepting references.");
    }
    switch(from->mQueen->GetKing()->GetLightType()) {
    case ZKing::LT_Moon: ++mMoonRefCount; break;
    case ZKing::LT_Sun:  ++mSunRefCount;  break;
    case ZKing::LT_Fire:
    default:		 ++mFireRefCount; break;
    }
    ++mRefCount;
    ++mReverseRefs[from];
    mQueen->SubjectRefCntUnlock();
    // Stamp(FID());
  }
  return mRefCount;
}

void ZGlass::dec_ref_count(hpZGlass2Int_i& i, UShort_t n)
{
  switch(i->first->mQueen->GetKing()->GetLightType()) {
  case ZKing::LT_Moon: mMoonRefCount -= n; break;
  case ZKing::LT_Sun:  mSunRefCount  -= n; break;
  case ZKing::LT_Fire:
  default:	       mFireRefCount -= n; break;
  }
  mRefCount -= n;
  i->second -= n;
}

Short_t ZGlass::DecRefCount(ZGlass* from, UShort_t n)
{
  // Called to notify *this* that it is no longer referenced by lens from.

  static const Exc_t _eh("ZGlass::DecRefCount ");

  if(mQueen && from->mQueen) {
    mQueen->SubjectRefCntLock();

    hpZGlass2Int_i i = mReverseRefs.find(from);
    if(i == mReverseRefs.end()) {
      mQueen->SubjectRefCntUnlock();
      ISerr(_eh + Identify() + " not referenced by " + from->Identify() + ".");
      return mRefCount;
    }
    if(n > i->second) {
      ISwarn(_eh + GForm("%s, called by %s: mismatch %d > %d.", Identify().Data(),
			 from->Identify().Data(), n, i->second));
      n = i->second;
    }

    dec_ref_count(i, n);

    if(i->second <= 0) mReverseRefs.erase(i);
    if(mRefCount == 0 && mQueen) mQueen->ZeroRefCount(this);
    mQueen->SubjectRefCntUnlock();
    // Stamp(FID());
  }
  return mRefCount;
}

Short_t ZGlass::IncEyeRefCount()
{
  return ++mEyeRefCount;
}

Short_t ZGlass::DecEyeRefCount()
{
  return --mEyeRefCount;
}

/**************************************************************************/

void ZGlass::ClearLinks()
{
  lppZGlass_t link_refs;
  CopyLinkRefs(link_refs);
  int n = 0;
  for(lppZGlass_i i=link_refs.begin(); i!=link_refs.end(); ++i) {
    if(**i) {
      (**i)->DecRefCount(this);
      **i = 0;
      ++n;
    }
  }
  if(n) StampLink();
}

void ZGlass::ClearAllReferences()
{
  ClearLinks();
}

Int_t ZGlass::RemoveReferencesTo(ZGlass* lens)
{
  Int_t n = remove_references_to(lens);
  if(n) lens->DecRefCount(this, n);
  return n;
}

/**************************************************************************/

ZGlass* ZGlass::GetLinkByName(const TString& link_name)
{
  // Returns glass pointed to by link with name link_name.
  // Attempts link_name == LinkMemberName and
  // link_name == Class::LinkMemberName.
  // Throws an exception (TString) if link does not exist.
  // In principle could have a map link-name->link-specs.

  // Should go in reverse direction !!!!
  // Locking doesn't make much sense.

  static const Exc_t _eh("ZGlass::GetLinkByName ");

  lLinkRep_t lr; CopyLinkReps(lr);
  for(lLinkRep_i i=lr.begin(); i!=lr.end(); ++i) {
    if(link_name == i->fLinkInfo->fName || link_name == i->fLinkInfo->FullName())
      return i->fLinkRef;
  }
  throw(_eh + GForm("link '%s' does not exist in '%s' (id=%u)",
		    link_name.Data(), GetName(), GetSaturnID()));
}

ZGlass* ZGlass::FindLensByPath(const TString& url, bool throwp)
{
  static const Exc_t _eh("ZGlass::FindLensByPath ");
  using namespace GledNS;

  list<url_token> tokens;
  tokenize_url(url, tokens);
  ZGlass* g = this;
  try {
    for(list<url_token>::iterator i=tokens.begin(); i!=tokens.end(); ++i) {
      assert(g != 0);
      switch (i->type()) {
      case url_token::link_sel: {
	ZGlass* lnk = g->GetLinkByName(*i);
	if(lnk == 0)
	  throw(_eh + "link '" + *i + "' of lens '" + g->Identify() + "' not set.");
	g = lnk;
	break;
      }
      case url_token::list_sel: {
	AList* l = dynamic_cast<AList*>(g);
	if(l == 0) {
	  throw(_eh + "url element '" + g->Identify() + "' is not a list.");
	}
	ZGlass* elm = l->GetElementByName(*i);
	if(elm == 0)
	  throw(_eh + "list " + g->Identify() + " does not have element '" + *i + "'.");
	g = elm;
	break;
      }
      default:
	throw(_eh + "unknown token type.");
	break;
      }
    }
  }
  catch(Exc_t& exc) {
    if(throwp) {
      ISerr(exc);
      throw;
    } else
      return 0;
  }
  return g;
}

/**************************************************************************/
// RayAbsorber

void ZGlass::register_ray_absorber(RayAbsorber* ra)
{
  GMutexHolder rdlck(mReadMutex);
  if(pspRayAbsorber == 0)
    pspRayAbsorber = new set<RayAbsorber*>;
  pspRayAbsorber->insert(ra);
}

void ZGlass::unregister_ray_absorber(RayAbsorber* ra)
{
  GMutexHolder rdlck(mReadMutex);
  if(pspRayAbsorber != 0)
    pspRayAbsorber->erase(ra);
}

/**************************************************************************/
// YNameChangeCB

void ZGlass::register_name_change_cb(ZGlass::NameChangeCB* nccb)
{
  if(mGlassBits & ZGlassBits::kFixedName) return;
  GMutexHolder rdlck(mReadMutex);
  if(pspNameChangeCB == 0)
    pspNameChangeCB = new set<NameChangeCB*>;
  pspNameChangeCB->insert(nccb);
}

void ZGlass::unregister_name_change_cb(ZGlass::NameChangeCB* nccb)
{
  GMutexHolder rdlck(mReadMutex);
  if(pspNameChangeCB != 0)
    pspNameChangeCB->erase(nccb);
}

/**************************************************************************/
// Stamping

TimeStamp_t ZGlass::Stamp(FID_t fid, UChar_t eye_bits)
{
  TimeStamp_t stamp = ++mTimeStamp;
  IF_ZGLASS_CHANGE_RAY(this, RayNS::RQN_change, stamp, fid, eye_bits) {
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t ZGlass::StampLink(FID_t fid)
{
  TimeStamp_t stamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_link_change, stamp, fid,
		Ray::EB_StructuralChange) {
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

/**************************************************************************/

void ZGlass::SetUseDLRec(Bool_t state)
{
  {
    GLensReadHolder rdlck(this);
    if(bUseDispList != state)
      SetUseDispList(state);
  }
  AList* l = dynamic_cast<AList*>(this);
  if(l != 0) {
    lpZGlass_t c; l->CopyList(c);
    for(lpZGlass_i i=c.begin(); i!=c.end(); ++i) {
      if((*i)->mQueen == mQueen)
	(*i)->SetUseDLRec(state);
    }
  }
}

/**************************************************************************/
/**************************************************************************/
// ZLinkBase
/**************************************************************************/
/**************************************************************************/

ID_t ZLinkBase::get_id()
{
  return (ID_t) ((char*)_lens - (char*)0);
}

void ZLinkBase::set_id(ID_t id)
{
  char* ptr = 0; ptr += id;
  _lens = (ZGlass*)ptr;
}


void ZLinkBase::Streamer(TBuffer &b)
{
  if(b.IsReading()) {
    ID_t id;  b >> id;
    char* ptr = 0; ptr += id;
    _lens = (ZGlass*)ptr;
  } else {
    ID_t id = _lens ? _lens->GetSaturnID() : 0;
    b << id;
  }
}

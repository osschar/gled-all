// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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

#include <Glasses/ZList.h>
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

ClassImp(ZGlass)

/**************************************************************************/

void ZGlass::_init()
{
  mSaturn = 0; mQueen = 0;
  mGlassBits = 0; mSaturnID=0;
  mGuard = 0;
  bMIRActive = true; bAcceptRefs = true;
  mRefCount = mMoonRefCount = mSunRefCount = mFireRefCount = mEyeRefCount = 0; 
  mTimeStamp = mStampReqTring = 0;
  pSetYNameCBs = 0;
  mStamp_CB = mStampLink_CB = 0; mStamp_CBarg = mStampLink_CBarg = 0;

  bUseDispList = false;
}

/**************************************************************************/

/**************************************************************************/
// Hairy Inlines
/**************************************************************************/

// inline /// used to be in the header ... gave segvs with cint
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

void ZGlass::reference_all() {
  lppZGlass_t link_refs;
  CopyLinkRefs(link_refs);
  for(lppZGlass_i i=link_refs.begin(); i!=link_refs.end(); ++i) {
    if(**i) {
      try {
	(**i)->IncRefCount(this);
      }
      catch(string exc) {
	**i = 0;
      }
    }
  }
}

void ZGlass::unreference_all() {
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
  // Removes all references to *lens*.
  // Virtual ... lists overrirde it but also call the ZGlass version
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

string ZGlass::Identify() const
{
  return string(GForm("'%s'[%d]", mName.Data(), mSaturnID));
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
  return GThread::get_mir();
}

ZMIR* ZGlass::assert_MIR_presence(const string& header, int what)
{
  ZMIR* mir = GThread::get_mir();
  if(mir == 0) {
    throw(header + "must be called via a MIR.");
  }
  if((what & MC_IsFlare) && mir->HasRecipient()) {
    throw(header + "must be called via a flared MIR.");
  }
  if((what & MC_IsBeam) && !mir->HasRecipient()) {
    throw(header + "must be called via a beamed MIR.");
  }
  if((what & MC_HasResultReq) && !mir->HasResultReq()) {
    throw(header + "must be called with a result request set.");
  }
  return mir;
}

ZMIR* ZGlass::suggest_MIR_presence(const string& header, int what)
{
  ZMIR* mir = GThread::get_mir();
  if(mir == 0) {
    ISwarn(header + "should be called via a MIR.");
    return 0;
  }
  if((what & MC_IsFlare) && mir->HasRecipient()) {
    ISwarn(header + "should be called via a flared MIR.");
  }
  if((what & MC_IsBeam) && !mir->HasRecipient()) {
    ISwarn(header + "should be called via a beamed MIR.");
  }
  if((what & MC_HasResultReq) && !mir->HasResultReq()) {
    ISwarn(header + "should be called with a result request set.");
  }
  return mir;
}

void ZGlass::warn_caller(const string& warning)
{
  ZMIR* mir = GThread::get_mir();
  if(mir && mir->Caller) {
    auto_ptr<ZMIR> wrn( mir->Caller->S_Warning(warning.c_str()) );
    wrn->SetRecipient(mir->Caller->HostingSaturn());
    mSaturn->ShootMIR(wrn);
  } else {
    ISwarn(warning);
  }
}

/**************************************************************************/
// YNameChangeCB

typedef set<ZGlass::YNameChangeCB*>		spYNameChangeCB_t;	
typedef set<ZGlass::YNameChangeCB*>::iterator	spYNameChangeCB_i;	

void ZGlass::register_name_change_cb(ZGlass::YNameChangeCB* rec)
{
  if(mGlassBits & ZGlassBits::kFixedName) return;
  ReadLock();
  if(pSetYNameCBs == 0)
    pSetYNameCBs = new set<YNameChangeCB*>;
  pSetYNameCBs->insert(rec);
  ReadUnlock();
}

void ZGlass::unregister_name_change_cb(ZGlass::YNameChangeCB* rec)
{
  ReadLock();
  if(pSetYNameCBs != 0)
    pSetYNameCBs->erase(rec);
  ReadUnlock();
}

/**************************************************************************/

void ZGlass::SetName(const Text_t* n)
{
  if(mGlassBits & ZGlassBits::kFixedName) {
    Stamp(FID());
    throw(string("ZGlass::SetName lens has FixedName bit set"));
  }
  string name(n);
  WriteLock();
  if(pSetYNameCBs != 0) {
    for(set<YNameChangeCB*>::iterator i=pSetYNameCBs->begin(); i!=pSetYNameCBs->end(); ++i) {
      (*i)->y_name_change_cb(this, name);
    }
  }
  mName = name.c_str();
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

ZMirFilter* ZGlass::GetGuard() const
{
  ReadLock();
  ZMirFilter* _ret = mGuard;
  ReadLock();
  return _ret;
}

void ZGlass::SetGuard(ZMirFilter* guard)
{
  // Might want to read-lock if LOCK_SET_METHS is true in project7.
  set_link_or_die((ZGlass*&)mGuard, guard, FID());
}

/**************************************************************************/

ZGlass::~ZGlass()
{
  // !!!!! Should unref all links; check zlist, too
  delete pSetYNameCBs;
}

/**************************************************************************/

void ZGlass::UpdateGlassView()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_change, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
  if(mStamp_CB) mStamp_CB(this, mStamp_CBarg);
}

void ZGlass::UpdateAllViews()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_change, mTimeStamp, FID_t(0,0)));
    mQueen->EmitRay(ray);
  }
  if(mStamp_CB) mStamp_CB(this, mStamp_CBarg);
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

  static const string _eh("ZGlass::IncRefCount ");

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

  static const string _eh("ZGlass::DecRefCount ");

  if(mQueen && from->mQueen) {
    mQueen->SubjectRefCntLock();
    
    hpZGlass2Int_i i = mReverseRefs.find(from);
    if(i == mReverseRefs.end()) {
      mQueen->SubjectRefCntUnlock();
      ISerr(_eh + Identify() + " not referenced by " + from->Identify() + ".");
    }
    if(n > i->second) {
      ISwarn(_eh + GForm("%s, called by %s: mismatch %d > %d.", Identify().c_str(),
			 from->Identify().c_str(), n, i->second));
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

ZGlass* ZGlass::GetLinkByName(const Text_t* link_name)
{ return GetLinkByName(string(link_name)); }

ZGlass* ZGlass::GetLinkByName(const string& link_name)
{
  // Returns glass pointed to by link with name link_name.
  // Attempts link_name == LinkMemberName and
  // link_name == Class::LinkMemberName.
  // Throws an exception (string) if link does not exist.
  // In principle could have a map link-name->link-specs.

  // Should go in reverse direction !!!!
  // Locking doesn't make much sense.

  lLinkRep_t lr; CopyLinkReps(lr);
  for(lLinkRep_i i=lr.begin(); i!=lr.end(); ++i) {
    if(link_name == i->fLinkInfo->fName || link_name == i->fLinkInfo->FullName())
      return i->fLinkRef;
  }
  throw(string(GForm("ZGlass::GetLinkByName link '%s' does not exist in '%s' (id=%u)",
		     link_name.c_str(), GetName(), GetSaturnID())));
}

ZGlass* ZGlass::FindLensByPath(const Text_t* url)
{ return FindLensByPath(string(url)); }

ZGlass* ZGlass::FindLensByPath(const string& url)
{
  static const string _eh("ZGlass::FindLensByPath ");
  using namespace GledNS;

  list<url_token> l;
  tokenize_url(url, l);
  ZGlass* g = this;
  try {
    for(list<url_token>::iterator i=l.begin(); i!=l.end(); ++i) {
      if(g == 0) {
	// !!! give more details
	throw(_eh + "null lens; probably a link is not set");
      }
      switch (i->type()) {
      case url_token::link_sel: {
	g = g->GetLinkByName(*i);
	break;
      }
      case url_token::list_sel: {
	ZList* l = dynamic_cast<ZList*>(g);
	if(l == 0) {
	  // !!! give more details
	  throw(_eh + "url element is not a list");
	}
	g = l->GetElementByName(*i);
	break;
      }
      default:
	ISerr(_eh + "unknown token type. Attempting to ignore ...");
	break;
      }
    }
  }
  catch(string exc) {
    ISerr(exc);
    throw;
  }

  return g;
}

/**************************************************************************/

TimeStamp_t ZGlass::Stamp(FID_t fid, UChar_t eye_bits)
{
  ++mTimeStamp;
  if(mQueen && mQueen->GetStamping() && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_change, mTimeStamp, fid, eye_bits));
    mQueen->EmitRay(ray);
  }
  if(mStamp_CB) mStamp_CB(this, mStamp_CBarg);

  return mTimeStamp;
}

TimeStamp_t ZGlass::StampLink(FID_t fid)
{
  ++mTimeStamp;
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_link_change, mTimeStamp, fid,
		    Ray::EB_StructuralChange));
    mQueen->EmitRay(ray);
  }
  if(mStampLink_CB) mStampLink_CB(this, mStampLink_CBarg);

  return mTimeStamp;
}

void ZGlass::SetStamp_CB(zglass_stamp_f foo, void* arg)
{
  mStamp_CB = foo; mStamp_CBarg = arg;
}
void ZGlass::SetStampLink_CB(zglass_stamp_f foo, void* arg)
{
  mStampLink_CB = foo; mStampLink_CBarg = arg;
}

/**************************************************************************/

Int_t ZGlass::RebuildAllRefs(An_ID_Demangler* idd)
{
  return RebuildLinkRefs(idd);
}

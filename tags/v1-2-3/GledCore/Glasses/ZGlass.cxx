// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// ZGlass
//
// Base of Gled enabled classes.
// Provides infrastructure for integration with the Gled system.
//
// mGlassBits: collection of flags that allow (optimised) handling of lenses.

#include "ZGlass.h"
#include "ZGlass.c7"

#include <Ephra/Saturn.h>
#include <Glasses/ZList.h>
#include <Glasses/ZQueen.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZMirFilter.h>
#include <Stones/ZMIR.h>
#include <Stones/ZComet.h>

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
  mSaturn = 0; mQueen = 0; mMir = 0;
  mGlassBits = 0; mSaturnID=0;
  mGuard = 0;
  bMIRActive = true; bAcceptRefs = true;
  mRefCount = mMoonRefCount = mSunRefCount = mFireRefCount = 0; 
  mTimeStamp = mStampReqTring = 0;
  pSetYNameCBs = 0;
  mStamp_CB=mStampLink_CB=0; mStamp_CBarg=mStampLink_CBarg=0;
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

void ZGlass::remove_references_to(ZGlass* lens)
{
  // Removes all references to *lens*.
  // Heavily virtual ... lists overrirde it but also call the ZGlass version
  // for unreferencing of links.
  // Calls DecRefCount and emits Rays.

  lppZGlass_t link_refs;
  CopyLinkRefs(link_refs);
  int n = 0;
  for(lppZGlass_i i=link_refs.begin(); i!=link_refs.end(); ++i) {
    if(**i == lens) {
      (**i)->DecRefCount(this);
      **i = 0;
      ++n;
    }
  }
  if(n) StampLink();
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

void ZGlass::assert_MIR_presence(const string& header, int what)
{
  if(mMir == 0) {
    throw(header + "must be called via a MIR.");
  }
  if((what & MC_IsFlare) && mMir->HasRecipient()) {
    throw(header + "must be called via a flared MIR.");
  }
  if((what & MC_IsBeam) && !mMir->HasRecipient()) {
    throw(header + "must be called via a beamed MIR.");
  }
  if((what & MC_HasResultReq) && !mMir->HasResultReq()) {
    throw(header + "must be called with result request set.");
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
    Stamp(LibID(), ClassID());
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
  Stamp(LibID(), ClassID());
  WriteUnlock();
  if(mMir && mQueen == this && ! mMir->HasRecipient()) {
    mQueen->BasicQueenChange(*mMir);
  }
}

void ZGlass::SetTitle(const Text_t* t)
{
  WriteLock();
  mTitle = t;
  Stamp(LibID(), ClassID());
  WriteUnlock();
  if(mMir && mQueen == this && ! mMir->HasRecipient()) {
    mQueen->BasicQueenChange(*mMir);
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
  WriteLock();
  try { set_link_or_die((ZGlass*&)mGuard, guard, LibID(), ClassID()); }
  catch(...) { WriteUnlock(); throw; }
  WriteUnlock();
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
  if(mQueen) {
    Ray r(Ray::RQN_change, mTimeStamp, this, LibID(), ClassID());
    mQueen->EmitRay(r, true);
  }
  if(mStamp_CB) mStamp_CB(this, mStamp_CBarg);
}

void ZGlass::UpdateAllViews()
{
  if(mQueen) {
    Ray r(Ray::RQN_change, mTimeStamp, this, LID_t(0), CID_t(0));
    mQueen->EmitRay(r, true);
  }
  if(mStamp_CB) mStamp_CB(this, mStamp_CBarg);
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
    case ZKing::LT_Moon:
      ++mMoonRefCount; ++mRefCount; mReverseRefs.push_front(from);
      break;
    case ZKing::LT_Sun:
      ++mSunRefCount; ++mRefCount; mReverseRefs.push_back(from);
      break;
    case ZKing::LT_Fire:
      ++mFireRefCount; ++mRefCount; mReverseRefs.push_back(from);
      break;
    default:
      mQueen->SubjectRefCntUnlock();
      throw(_eh + "King in undefined state. Ignoring.");
    }
    mQueen->SubjectRefCntUnlock();
    // Stamp(LibID(), ClassID());
  }
  return mRefCount;
}

Short_t ZGlass::DecRefCount(ZGlass* from)
{
  // Called to notify *this* that it is no longer referenced by lens from.

  if(mQueen && from->mQueen) {
    mQueen->SubjectRefCntLock();
    switch(from->mQueen->GetKing()->GetLightType()) {
    case ZKing::LT_Moon: {
      lpZGlass_i i = find(mReverseRefs.begin(), mReverseRefs.end(), from);
      if(i != mReverseRefs.end()) {
	--mMoonRefCount; --mRefCount;
	mReverseRefs.erase(i);
      }
      break;
    }
    case ZKing::LT_Sun: {
      lpZGlass_ri i = find(mReverseRefs.rbegin(), mReverseRefs.rend(), from);
      if(i != mReverseRefs.rend()) {
	--mSunRefCount; --mRefCount;
	mReverseRefs.erase((++i).base());
      }
      break;
    }
    case ZKing::LT_Fire: {
      lpZGlass_ri i = find(mReverseRefs.rbegin(), mReverseRefs.rend(), from);
      if(i != mReverseRefs.rend()) {
	--mFireRefCount; --mRefCount;
	mReverseRefs.erase((++i).base());
      }
      break;
    }
    default:
      ISerr("ZGlass::IncRefCount King in undefined state");
    }
    if(mRefCount == 0 && mQueen) mQueen->ZeroRefCount(this);
    mQueen->SubjectRefCntUnlock();
    // Stamp(LibID(), ClassID());
  }
  return mRefCount;
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

  lLinkSpec_t ls; CopyLinkSpecs(ls);
  for(lLinkSpec_i i=ls.begin(); i!=ls.end(); ++i) {
    if(link_name == i->fLinkName || link_name == i->full_name())
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

// All stamp functions should be *inline*!!

TimeStamp_t ZGlass::Stamp(LID_t lid, CID_t cid)
{
  ++mTimeStamp;
  if(mQueen) {
    Ray r(Ray::RQN_change, mTimeStamp, this, lid, cid);
    mQueen->EmitRay(r);
  }
  if(mStamp_CB) mStamp_CB(this, mStamp_CBarg);

  return mTimeStamp;
}

TimeStamp_t ZGlass::StampLink(LID_t lid, CID_t cid)
{
  ++mTimeStamp;
  if(mQueen) {
    Ray r(Ray::RQN_link_change, mTimeStamp, this, lid, cid);
    mQueen->EmitRay(r);
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

Int_t ZGlass::RebuildAll(An_ID_Demangler* idd)
{
  return RebuildLinks(idd);
}

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//________________________________________________________________________
// ZQueen
//
// A Queen controls its portion of the ID space.
// Glasses are adopted/instantiated and enlightened by it.
// Queens (like Kings) are exported to all Moons but are not
// necesarily activated immediately (controled by bMandatory).
// Queen's status is given by bRuling.
//
// As queens are always at least partially exposed, MIRs with alpa=queen
// are always broadcasted to all Moons. Exported methods that should
// not be called for an inactive queen are prefixed with if-not-ruling-return.
//________________________________________________________________________

#include "ZQueen.h"
#include <Stones/ZMIR.h>
#include <Stones/ZComet.h>
#include <Ephra/Saturn.h>
#include <Gled/GledNS.h>

ClassImp(ZQueen)

/**************************************************************************/

void ZQueen::_init()
{
  bMandatory = bRuling = bAwaitingSceptre = false;
  bStamping = true;

  mAuthMode = AM_LensThenQueen;
  mAlignment = A_Good;
  mMapNoneTo = ZMirFilter::R_Deny;
  mProtector = 0;

  mDeps = 0; mOrphans = 0;
}

/**************************************************************************/

void ZQueen::Bootstrap()
{
  // Initializes a brand new queen.
  // Called from ZKing.
  // Not applicable for activation or instantiation of not Ruling Queens.

  CheckIn(this);

  ZHashList* l = new ZHashList(GForm("Deps of %s", mName.Data()));
  CheckIn(l); SetDeps(l); l->SetMIRActive(false);

  l = new ZHashList(GForm("Orphans of %s", mName.Data()));
  CheckIn(l); SetOrphans(l); l->SetMIRActive(false);

  bRuling = true;
}

void ZQueen::BlessMIR(ZMIR& mir) throw(string)
{
  // should check that the mir is valid and within deps.
  // If whorequeen should accept new ones and append them.
  // If light-queen with OpenedDeps should append with broadcast
  // PRIOR to returning from this method.
  // Called from Saturn::Unfold() prior to locking and execution.
  // Errors/non-conformance should be reported by throwing an exception.
  //
  // Oh yess ... btw ... queens can as well block all execs to their
  // subjects. Eg, when bMIRActive of alpha is false.
  // Per queen access control can be implemented here.

  // !!!! fist check, now needed ... bRuling ... is me alpha?

  static string _eh("ZQueen::BlessMIR() ");

  if(!bRuling && mir.Alpha != this)
    throw(_eh + "spooky ... not ruling, but alpha demangled");
  
  if(!mir.Alpha->GetMIRActive())
    throw(_eh + "alpha [" + mir.Alpha->GetName() + "] not MIR active");

  // !!!! Mising dependency check

  /**************************************************************************/

  // Authorization

  // Allow everything if UseAuth is false
  if(mSaturn->GetSaturnInfo()->GetUseAuth() == false) {
    return;
  }

  // Always allow SunAbsolute access
  if(mir.Caller->HasIdentity(mSaturn->mSunInfo->GetPrimaryIdentity())) {
    return;
  }

  UChar_t result = ZMirFilter::R_None;
  if(mAuthMode != AM_None) {
    ZMirFilter* fs[2] = { 0, 0 };
    switch(mAuthMode) {
    case AM_Queen: fs[0] = mProtector; break;
    case AM_Lens:  fs[0] = mir.Alpha->mGuard; break;
    case AM_QueenThenLens: fs[0] = mProtector; fs[1] = mir.Alpha->mGuard; break;
    case AM_LensThenQueen: fs[0] = mir.Alpha->mGuard; fs[1] = mProtector; break;
    }
    for(int i=0; i<2; ++i) {
      if(fs[i] != 0) {
	result |= fs[i]->FilterMIR(mir);
	if(mAlignment == A_Good && result & ZMirFilter::R_Allow) return;
	if(mAlignment == A_Evil && result & ZMirFilter::R_Deny)  break;
      }
    }
  }
  if( result & ZMirFilter::R_Deny ||
     (result == ZMirFilter::R_None && mMapNoneTo == ZMirFilter::R_Deny))
    {
      throw(_eh + "access denied");
    }
}

/**************************************************************************/

ID_t ZQueen::ReserveID()
{
  mExecMutex.Lock();
  ID_t new_id = 0;
  if(mMaxUsedID < mMaxID) {
    new_id = ++mMaxUsedID;
  } else {
    // Here need some traversal over Saturn->hash-lookup.
    // If i would keep numbers of actually used ids (and some ranges)
    // this could be partially optimal.
  }
  mExecMutex.Unlock();
  if(new_id == 0)
    throw(string("ZQueen::ReserveID no free ids ... partially coded though"));
  return new_id;
}

ID_t ZQueen::CheckIn(ZGlass* glass)
{
  ID_t new_id = 0;
  try {
    new_id =  ReserveID();
    glass->mQueen = this;
    glass->SetStamps(1);
    mSaturn->Enlight(glass, new_id);
  }
  catch(string s) {
    ISerr(GForm("ZQueen::CheckIn enlightenment failed: %s", s.c_str()));
    glass->mQueen = 0;
  }
  return new_id;
}

void ZQueen::CheckOut(ZGlass* glass)
{
  // !!!!!! this is fooed ... Queens must have special mechanism for deletion
  // of lenses. Note ... eyes are pointer-centric, not id-centric.
  // Further ... deletion itself should be delayed and performed
  // on Saturn some time *after* endarkment._eh + 
  // Perform with special message type that is only execed on level lower;
  // that call should then decide what to send furter (remove or migrate)
  // Which means there is a need for DumpQueen

  mExecMutex.Lock();
  ID_t ex_id = glass->mSaturnID;

  try {
    mSaturn->Endark(glass); 
  }
  catch(string s) {
    ISerr(GForm("ZQueen::CheckOut endarkenment failed: %s", s.c_str()));
  }

  mExecMutex.Unlock();
}

//void ZQueen::CheckInWithAttach(ZGlass* glass) {}

/**************************************************************************/
// Instantiators
/**************************************************************************/

ZGlass* ZQueen::instantiate(LID_t lid, CID_t cid,
                            const Text_t* name, const Text_t* title)
{
  ZGlass* g = GledNS::ConstructGlass(lid, cid);
  if(g == 0) throw(string("ZQueen::instantiate failed lens instantiation"));
  if(name)  g->mName  = name;
  if(title) g->mTitle = title;
  CheckIn(g);
  return g;
}

namespace {
  int _mefid_size = sizeof(LID_t) + sizeof(CID_t) + sizeof(MID_t);
}

ZGlass* ZQueen::InstantiateWAttach(ZGlass* attach_to, ZGlass* attach_gamma,
		   LID_t att_lid, CID_t att_cid, MID_t att_mid,
		   LID_t new_lid, CID_t new_cid,
		   const Text_t* name, const Text_t* title)
{
  // Instantiates a glass of FID(lid, cid) and attaches it to attach_to
  // by using last part of the message, which should be a properly
  // formulated Ctx call.
  // Ctx for this call is:
  // alpha ~ attach_to, beta ~ new glass, gamma ~ attach_gamma

  static string _eh("ZQueen::InstantiateWAttach ");

  if(!bRuling) return 0;
  if(attach_to->GetQueen() != this)
    throw(_eh + "can only attach to my own subjects");
  if(mMir == 0)
    throw(_eh + "must be called via MIR");

  ZGlass* g = GledNS::ConstructGlass(new_lid, new_cid);
  if(g == 0) throw(_eh + "failed lens instantiation");

  CheckIn(g);
  if(name)  g->SetNameTitle(name, title);

  // overwrite the Mir ... not too good.
  mMir->Alpha = attach_to;
  mMir->Beta  = g;
  mMir->Lid = att_lid; mMir->Cid = att_cid; mMir->Mid = att_mid;
  try {
    mSaturn->ExecMIR(*mMir);
  }
  catch(string exc) {
    // should delete/checkout g?
    // throw(_eh + "attachment failed: " + exc); // !!! no good until delete works
    ISwarn(_eh + "attachment failed: " + exc);
  }
  if(g->mRefCount <= 0) {
    // attach failed ... hmmmph?
    // Perhaps should delete it?
    ZeroRefCount(g);
  }
  return g;
}

ZGlass* ZQueen::IncarnateWAttach(ZGlass* attach_to, ZGlass* attach_gamma)
{
  // Incarnates a new glass from the buffer and then attaches it in the
  // same manner as the above method.
  // Links MUST NOT be set as they will not be rebuilt.
  // !!!! Probably should set them to zero prior to CheckIn.
  // !!!!! This half-cooked anyway ... no mefid stuff.

  static string _eh("ZQueen::IncarnateWAttach ");

  if(!bRuling) return 0;
  if(mMir == 0) throw(_eh + "must be called via MIR");

  ZGlass* g = GledNS::StreamGlass(*mMir);
  if(g == 0) throw(_eh + "failed unstreaming lens");

  CheckIn(g);
  mMir->Alpha = attach_to;
  mMir->Beta  = g;
  mSaturn->ExecMIR(*mMir);
  if(g->mRefCount <= 0) {
    // attach failed ... hmmmph?
    // Perhaps should delete it?
    ZeroRefCount(g);
  }
  return g;
}

/**************************************************************************/
// Orphans
/**************************************************************************/

void ZQueen::ZeroRefCount(ZGlass* glass)
{
  // Called by ZGlass upon hitting refcount of 0.
  // No checks done.
  // This operation always performed locally, as refcount is a local property.
  mOrphans->Add(glass);
}

void ZQueen::CleanOrphanage()
{
  if(!bRuling) return;
  mOrphans->mListMutex.Lock();
  lpZGlass_t l; mOrphans->Copy(l);
  for(lpZGlass_i i=l.begin(); i!=l.end(); ++i) {
    if((*i)->mRefCount > 1) {
      mOrphans->Remove(*i);
    }
    /*
    else if((*i)->mRefCount == 1) {
      (*i)->mRefCount = -1;
      mOrphans->Remove(*i);
      CheckOut(*i);
    }
    */
  }
  mOrphans->mListMutex.Unlock();
}

/**************************************************************************/
// Reflections
/**************************************************************************/

// All Reflections are operated by King of the Queen.
// State variables and list of reflectors are also managed by the King.

void ZQueen::CreateReflection(TBuffer& buf)
{
  // Virtual. Creates reflection that can be invoked to produce a copy
  // of queen-space on other Saturn.
  // ZQueen itself just creates and streams a ZComet.

  ZComet* c = MakeComet();
  c->Streamer(buf);
  delete c;
}

void ZQueen::InvokeReflection(TBuffer& buf)
{
  // Virtual. Invokes reflection created by a higher Saturn and thus
  // begins mirroring the object-space.
  // For ZQueen this involves unstreaming of ZComet and calling UnfoldFrom().

  ZComet c; c.Streamer(buf); c.UseSaturn(mSaturn); c.bVerbose = false;
  c.RebuildGraph();
  UnfoldFrom(c);
}

void ZQueen::RevokeReflection()
{
  // Virtual. Cleans up queen-space, putting it back into non-ruling state.
}

/**************************************************************************/
// Comet ops
/**************************************************************************/

ZComet* ZQueen::MakeComet()
{
  // Produces Comet of type ST_Queen.
  // Contains all glasses ruled by the queen.
  // External references are streamed as IDs and can be reconstructed
  // at lower Moons.
  // Used for producing a snapshot of Queen-space to be sent to a Moon.

  ZComet* comet = new ZComet(GetName(), GForm("Comet[Queen] of %s", GetName()));
  comet->mType = ZComet::CT_Queen;
  comet->mQueen = this;

  mExecMutex.Lock();
  sID_i next_free = mFreeIDs.begin();
  ID_t  next_free_id = next_free!=mFreeIDs.end() ? *next_free : 0;
  for(ID_t id=mMinID; id<=mMaxUsedID; ++id) {
    if(next_free_id && id == next_free_id) {
      ++next_free;
      next_free_id = next_free!=mFreeIDs.end() ? *next_free : 0;
      continue;
    }
    ZGlass* g = mSaturn->DemangleID(id);
    comet->AddGlass(g);
  }
  mExecMutex.Unlock();

  return comet;
}

void ZQueen::Adopt(ZComet* comet)
{
  // Adopts a Comet of type ST_CometBag.
  // External references of a Comet are ignored.

  comet->RebuildGraph();
  mExecMutex.Lock();  
  UInt_t free = mMaxID - mMaxUsedID + mFreeIDs.size();
  UInt_t need = comet->mIDMap.size();
  if(free >= need) {
    for(mID2pZGlass_i i=comet->mIDMap.begin(); i!=comet->mIDMap.end(); ++i)
      CheckIn(i->second);
    for(lpZGlass_i i=comet->mTopLevels.begin(); i!=comet->mTopLevels.end(); ++i)
      Add(*i);
    for(lpZGlass_i i=comet->mOrphans.begin(); i!=comet->mOrphans.end(); ++i)
      mOrphans->Add(*i);
  } else {
    mExecMutex.Unlock();
    throw(string("ZQueen::Adopt(comet=") + comet->GetName() + ") ID shortage");
  }
  mExecMutex.Unlock();
}

void ZQueen::UnfoldFrom(ZComet& comet)
{
  // Unfolds a dormant (not bRuling) queen from a comet of type ST_Queen
  // and makes it a ruling queen.
  // The incoming queen and its Deps are replaced with already present ones.
  // Does NOT call CheckIn ... does it directly with simultaneous building of
  // free block list.
  // At the end calls AdUnfoldment on all lenses.

  assert(comet.mType == ZComet::CT_Queen && comet.mQueen != 0);

  mExecMutex.Lock();

  ID_t last_id = mMinID + 1;
  { // Reflect all lenses
    mID2pZGlass_i i = comet.mIDMap.begin();
    ++i; ++i; // Skip queen and deps; order created by ZQueen::Bootstrap()
    while(i != comet.mIDMap.end()) {
      i->second->mQueen = this;
      i->second->SetStamps(1);
      mSaturn->Reflect(i->second);
      while(i->first > ++last_id) mFreeIDs.insert(last_id);
      ++i;
    }
  }
  mMaxUsedID = last_id;
  mGuard     = comet.mQueen->mGuard;
  mProtector = comet.mQueen->mProtector;
  mOrphans   = comet.mQueen->mOrphans;

  lpZGlass_t subs; comet.mQueen->Copy(subs);
  mGlasses.clear();
  mItMap.clear();
  mSize = 0;
  for(lpZGlass_i s=subs.begin(); s!=subs.end(); ++s) {
    mGlasses.push_back(*s); ++mSize;
    mItMap[(*s)->GetName()] = --mGlasses.end();
  }

  bRuling = true;

  { // Call AdUnfoldment for all lenses
    mID2pZGlass_i i= comet.mIDMap.begin(); ++i; ++i; // Skip queen and deps.
    while(i != comet.mIDMap.end()) {
      i->second->AdUnfoldment();
      ++i;
    }
  }

  Stamp();
  StampLink();
  StampListRebuild();

  mExecMutex.Unlock();
  
  delete comet.mQueen->mDeps;
  delete comet.mQueen;
}

/**************************************************************************/
// Reflectors
/**************************************************************************/

void ZQueen::add_reflector(SaturnInfo* moon)
{
  lpSaturnInfo_i i = find(mReflectors.begin(), mReflectors.end(), moon);
  if(i == mReflectors.end()) {
    mReflectors.push_back(moon);
  }
}

void ZQueen::remove_reflector(SaturnInfo* moon)
{
  lpSaturnInfo_i i = find(mReflectors.begin(), mReflectors.end(), moon);
  if(i != mReflectors.end()) {
    mReflectors.erase(i);
  }
}


/**************************************************************************/
// Stamping
/**************************************************************************/

void ZQueen::EmitRay(Ray& ray)
{
  if(bStamping) mSaturn->Shine(ray);
}

/**************************************************************************/
// tmp
/**************************************************************************/

void ZQueen::ListAll()
{
  // attempt at looking at all subjects based on ids
  // else might be forced into having local hash.
  sID_i next_free = mFreeIDs.begin();
  ID_t  next_free_id = next_free!=mFreeIDs.end() ? *next_free : 0;
  for(ID_t id=mMinID; id<=mMaxUsedID; ++id) {
    if(next_free_id && id == next_free_id) {
      ++next_free;
      next_free_id = next_free!=mFreeIDs.end() ? *next_free : 0;
      continue;
    }
    ZGlass* g = mSaturn->DemangleID(id);
    cout <<"ID="<< id <<", "<< g->GetName() <<"["<< g->ZlassName() <<"]\n";
  }
}

/**************************************************************************/

#include "ZQueen.c7"

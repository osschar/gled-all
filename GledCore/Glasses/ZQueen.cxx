// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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
// Although queens are always at least partially exposed, MIRs with
// alpa=queen are *not* broadcasted to all Moons by the Saturn's mir
// processing code. Exported methods that should be called also at
// inactive queens must be pushed there by hand. This service is
// offered by the BroadcastMIRToNonRulingReflections() method. For
// example see ZQueen::AddDependency.
//
//________________________________________________________________________

#include "ZQueen.h"
#include <Stones/ZComet.h>
#include "ZQueen.c7"
#include "ZKing.h"
#include <Ephra/Saturn.h>
#include <Gled/GledNS.h>

#include <Gled/GThread.h>
#include <TSystem.h>

ClassImp(ZQueen)

/**************************************************************************/

void ZQueen::_init()
{
  bMandatory = false;
  bFollowDeps = false;
  bRuling = bAwaitingSceptre = false;

  mMinID = mMaxID = 0;
  mIDsUsed = mIDsPurged = mIDsFree;
  mAvgPurgLen = 32; mSgmPurgLen = 0.2;
  mPurgedMS = 1000; mDeletedMS = 5000;

  mZeroRCPolicy = ZRCP_Delete;

  bStamping = true;

  mAuthMode = AM_LensThenQueen;
  mAlignment = A_Good;
  mMapNoneTo = ZMirFilter::R_Deny;
  mProtector = 0;

  mDeps = 0; mOrphans = 0;
}

/**************************************************************************/

void ZQueen::bootstrap()
{
  // Initializes a new queen and creates mandatory lists.
  // Called from ZKing.
  // Not applicable for activation or instantiation of not Ruling Queens.

  static const string _eh("ZQueen::bootstrap ");

  assert(bRuling == false);
  assert(mIDMap.size() == 0 && mMinID && mMaxID > mMinID);
  assert(mPurgatory.size() == 0);  

  try {
    mQueen = this;
    SetStamps(1);
    mSaturn->Enlight(this, mMinID);
  } catch(string exc) {
    ISerr(_eh + "Queen enlightenment failed.");
    throw;
  }
  mIDMap.insert
    ( pair<ID_t,LensDetails*>
      (mSaturnID, produce_lens_details(mSaturnID, this))
    ).first;
  mCreationID = mSaturnID + 1;
  mIDsUsed   = 1;
  mIDsPurged = 0;
  mIDsFree   = mIDSpan - 1;

  ZHashList* l = new ZHashList(GForm("Deps of %s", mName.Data()));
  l->SetElementFID(ZQueen::FID());
  CheckIn(l); SetDeps(l); l->SetMIRActive(false);

  l = new ZHashList(GForm("Orphans of %s", mName.Data()));
  CheckIn(l); SetOrphans(l); l->SetMIRActive(false);

  bRuling = true;
}

void ZQueen::embrace_of_life(ZComet& comet)
{
  // Initializes *this* queen with unstreamed queen.
  // Copies Links and List contents, no ref-counting is performed.

  ZQueen* queen = comet.mQueen;
  assert(ZID() == queen->ZID());

  { // Re-stream the queen and read it back into *this*
    TBuffer qb(TBuffer::kWrite);
    queen->Streamer(qb);
    qb.SetReadMode(); qb.SetBufferOffset(0);
    ZHashList* ex_deps = mDeps;
    Streamer(qb);
    RebuildLinkRefs(&comet);
    RebuildListRefs(&comet);
    mDeps = ex_deps;
  }
}
/**************************************************************************/

bool ZQueen::has_free_ids(ID_t n_needed)
{
  if(n_needed + 1 > mIDsFree) {
    release_purgatory(n_needed - mIDsFree + 1);
  } else {
    if(mIDsPurged > ID_t((1 + mSgmPurgLen)*mAvgPurgLen + 1)) {
      release_purgatory(0);
    }
  }
  return (n_needed <= mIDsFree);
}

ID_t ZQueen::next_free_id(QueenIDMap_i i)
{
  static const string _eh("ZQueen::next_free_id ");

  bool wrapp = false;
  QueenIDMap_i j = i; ++j;
  while(1) {
    if(j != mIDMap.end()) {
      if(j->first > i->first + 1) break;
      i = j; ++j;
    } else {
      if(i->first < mMaxID) break;
      if(wrapp) throw(_eh + "looped twice over the end ... dying.");
      wrapp = true;
      i = mIDMap.begin(); j = i; ++j;
    }
  }
  // i holds one prior to free space; j is junk.
  return i->first + 1;
}

ID_t ZQueen::assign_id(ZGlass* lens)
{
  // Assigns an ID to lens.

  static const string _eh("ZQueen::assign_ID ");

  if(mCreationID == 0) {
    mCreationID = next_free_id(mIDMap.begin());
  }

  ID_t this_id = mCreationID ? mCreationID : next_free_id(mIDMap.begin());

  pair<QueenIDMap_i, bool> poo = mIDMap.insert
    ( pair<ID_t, LensDetails*>
        (this_id, produce_lens_details(this_id, lens))
    );
  QueenIDMap_i ins_pos = poo.first;
  ++mIDsUsed; --mIDsFree;
  mCreationID = mIDsFree ? next_free_id(ins_pos) : 0;
  Stamp(LibID(), ClassID());

  return this_id;
}

ZQueen::LensDetails* ZQueen::produce_lens_details(ID_t id, ZGlass* lens)
{
  return new LensDetails(lens);
}

void ZQueen::release_purgatory(ID_t n_needed)
{
  static const string _eh("ZQueen::release_purgatory ");

  ID_t n_cleared = 0;
  ID_t happy = ID_t((1 - mSgmPurgLen)*mAvgPurgLen);

  GTime now(GTime::I_Now);
  while(mIDsPurged && (n_needed || mIDsPurged > happy)) {

    ID_t id = mPurgatory.front();
    QueenIDMap_i i = mIDMap.find(id);
    assert(i != mIDMap.end());

    if(i->second->mState != LensDetails::LS_Dead)
      break;

    GTime dt = now - i->second->mDeletionTime;
    if(dt.ToMiliSec() < mDeletedMS)
      break;

    delete i->second;
    mIDMap.erase(i);
    mPurgatory.pop_front();
    --mIDsPurged; ++mIDsFree;
    if(n_needed) --n_needed;
    ++n_cleared;

  }

  if(n_cleared) {
    ISmess(_eh + Identify() + GForm(" cleared %d entries.", n_cleared));
  }

  // broadcast release_moon_purgatory
  if(n_cleared) {
    auto_ptr<ZMIR> moonir( S_release_moon_purgatory(n_cleared) );
    mSaturn->markup_posted_mir(*moonir, mSaturn->GetSaturnInfo());
    mSaturn->BroadcastMIR(*moonir, mReflectors);
  }
}

void ZQueen::release_moon_purgatory(ID_t n_to_release)
{
  static const string _eh("ZQueen::release_purgatory ");

  ID_t n_done = 0;
  while(n_done < n_to_release) {
    ID_t id = mPurgatory.front();
    mPurgatory.pop_front();
    QueenIDMap_i i = mIDMap.find(id);
    if(i != mIDMap.end()) {
      delete i->second;
      mIDMap.erase(i);
    } else {
      ISwarn("Safromastundhell.");
    }
    --mIDsPurged; ++mIDsFree;
    ++n_done;
  }
  ISmess(_eh + Identify() + GForm(" cleared %d entries.", n_done));
  Stamp(LibID(), ClassID());
}

/**************************************************************************/
// Lens ID/ptr handling
/**************************************************************************/

ZGlass* ZQueen::DemangleID(ID_t id)
{
  // This should serve to properly demangle external references for comets.
  // And check that the id is within the queen's dependencies.

  ZGlass* l = mSaturn->DemangleID(id);
  if(l && DependsOn(l->mQueen))
    return l;
  return 0;
}

/**************************************************************************/

ID_t ZQueen::CheckIn(ZGlass* lens)
{
  static const string _eh("ZQueen::CheckIn ");

  WriteLock();
  if(mKing->GetLightType() != ZKing::LT_Moon) {
    if( ! has_free_ids(1) ) {
      WriteUnlock();
      throw(_eh + "no ID space available.");
    }
  }
  ID_t new_id;
  try {
    new_id = assign_id(lens);
  }
  catch(string exc) {
    WriteUnlock();
    ISerr(_eh + "ID assignment failed: " + exc);
    throw;
  }
  try {
    lens->mQueen = this;
    lens->SetStamps(1);
    mSaturn->Enlight(lens, new_id);
  }
  catch(string exc) {
    WriteUnlock();
    ISerr(_eh + "enlightenment failed: " + exc);
    // THIS INDICATES A SERIOUS BUG: fail miseribly.
    fprintf(stderr, "%s detected inconsistency in Saturn ID management. "
	    "Stopping server.\n", _eh.c_str());
    mSaturn->Shutdown();
    throw;
  }
  WriteUnlock();
  return new_id;
}

/**************************************************************************/

void ZQueen::DepCheckMIR(ZMIR& mir)
{
  // Allow beta/gamma from queens that *this* depends on + queen
  // arguments if alpha is *this*.

  static const string _eh("ZQueen::DepCheckMIR ");

  if(mir.Beta) {
    if(! DependsOn(mir.Beta->mQueen) && (mir.Alpha!=this || mir.Beta!=mir.Beta->mQueen))
      {
	if(bFollowDeps) {
	  // !!!! missing code to add the dependency etc.
	} else {
	  throw(_eh + GForm("beta '%s', id=%d: dependency check failed.",
			    mir.Beta->GetName(), mir.BetaID));
	}
      }
  }
  if(mir.Gamma) {
    if(! DependsOn(mir.Gamma->mQueen) && (mir.Alpha!=this || mir.Gamma!=mir.Gamma->mQueen))
      {
	if(bFollowDeps) {
	  // !!!! missing code to add the dependency etc.
	} else {
	  throw(_eh + GForm("gamma '%s', id=%d: dependency check failed.",
			    mir.Gamma->GetName(), mir.GammaID));
	}
      }
  }
}

void ZQueen::BlessMIR(ZMIR& mir)
{
  // should check that the mir is valid and within deps.
  // If whorequeen should accept new ones and append them.
  // If light-queen with FollowDeps should append with broadcast
  // PRIOR to returning from this method.
  // Called from Saturn::Unfold() prior to locking and execution.
  // Errors/non-conformance should be reported by throwing an exception.
  //
  // Oh yess ... btw ... queens can as well block all execs to their
  // subjects. Eg, when bMIRActive of alpha is false.
  // Per queen access control can be implemented here.

  static string _eh("ZQueen::BlessMIR() ");

  if(!bRuling && mir.Alpha != this)
    throw(_eh + "spooky ... not ruling, but alpha demangled");
  
  if(!mir.Alpha->GetMIRActive())
    throw(_eh + "alpha '" + mir.Alpha->GetName() + "' not MIR active");

  // Dependency check
  DepCheckMIR(mir);

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
// Dependencies
/**************************************************************************/

void ZQueen::AddDependency(ZQueen* new_dep)
{
  static const string _eh("ZQueen::AddDependency ");

  assert_MIR_presence(_eh, ZGlass::MC_IsFlare);
}

Bool_t ZQueen::DependsOn(ZQueen* some_queen)
{
  // Returns true if *this* depends on some_queen, i.e. if my lenses
  // can reference lenses ruled by some_queen.

  return some_queen == this || some_queen == (ZQueen*)mSaturn->GetSunQueen() ||
    mDeps->Has(some_queen);
}

/**************************************************************************/
// Instantiators
/**************************************************************************/

ZGlass* ZQueen::instantiate(LID_t lid, CID_t cid,
			    const Text_t* name, const Text_t* title)
{
  ZGlass* g = GledNS::ConstructLens(lid, cid);
  if(g == 0) throw(string("ZQueen::instantiate failed lens instantiation"));
  if(name)  g->mName  = name;
  if(title) g->mTitle = title;
  CheckIn(g);
  return g;
}

ID_t ZQueen::InstantiateWAttach(ZGlass* attach_to, ZGlass* attach_gamma,
				LID_t att_lid, CID_t att_cid, MID_t att_mid,
				LID_t new_lid, CID_t new_cid,
				const Text_t* name, const Text_t* title)
{
  // Instantiates a glass of FID(lid, cid) and attaches it to attach_to
  // by using last part of the message, which should be a properly
  // formulated Ctx call.
  // Ctx for this call is:
  // alpha ~ attach_to, beta ~ new glass, gamma ~ attach_gamma
  // Returns ID to the caller (also via MirResult if set in MIR).

  static string _eh("ZQueen::InstantiateWAttach ");

  assert(bRuling);
  ZMIR* mir = assert_MIR_presence(_eh, ZGlass::MC_IsFlare);

  if(attach_to->GetQueen() != this)
    throw(_eh + "can only attach to my own subjects");

  ZGlass* lens = GledNS::ConstructLens(new_lid, new_cid);
  if(lens == 0) throw(_eh + "failed lens instantiation");

  try {
    CheckIn(lens);
  }
  catch(string exc) {
    delete lens;
    throw(_eh + exc);
  }
  if(name)  lens->SetName(name);
  if(title) lens->SetTitle(title);

  auto_ptr<ZMIR> att_mir( new ZMIR(attach_to, lens, attach_gamma) );
  att_mir->SetLCM_Ids(att_lid, att_cid, att_mid);
  att_mir->SetCaller(mir->Caller);
  try {
    // Should Bless? Yess ... but before lens construction ... then fix beta.
    // Or ... in this case ... could Request blessing by another queen.
    mSaturn->ExecMIR(att_mir);
  }
  catch(string exc) {
    // Attach failed ...
    ISwarn(_eh + "attachment failed: " + exc);
  }
  if(lens->mRefCount <= 0) {
    // Perhaps should delete it?
    ZeroRefCount(lens);
  }

  if(mir->HasResultReq()) {
    TBuffer b(TBuffer::kWrite);
    b << lens->mSaturnID;
    mSaturn->ShootMIRResult(b);
  }

  return lens->mSaturnID;
}

ID_t ZQueen::IncarnateWAttach(ZGlass* attach_to, ZGlass* attach_gamma,
			      LID_t att_lid, CID_t att_cid, MID_t att_mid)
{
  // Incarnates a new glass from the buffer and then attaches it in the
  // same manner as the above method.
  // Links and list contents are rebuilt in accordance with queen's dependency
  // state.

  static string _eh("ZQueen::IncarnateWAttach ");

  assert(bRuling);
  ZMIR* mir = assert_MIR_presence(_eh, ZGlass::MC_IsFlare);

  ZGlass* lens = GledNS::StreamLens(*mir);
  if(lens == 0) throw(_eh + "lens unstreaming failed.");

  { // Rebuild link/list refs
    lens->mQueen = this;
    int n_failed = lens->RebuildAllRefs(this);
    if(n_failed > 0) {
      ISwarn(_eh + GForm("missed %d referenced lenses.", n_failed));
    }
  }

  try {
    CheckIn(lens);
  }
  catch(string exc) {
    lens->unreference_all();
    delete lens;
    throw(_eh + exc);
  }

  auto_ptr<ZMIR> att_mir( new ZMIR(attach_to, lens, attach_gamma) );
  att_mir->SetLCM_Ids(att_lid, att_cid, att_mid);
  att_mir->SetCaller(mir->Caller);
  try {
    mSaturn->ExecMIR(att_mir);
  }
  catch(string exc) {
    // Attach failed ...
    ISwarn(_eh + "attachment failed: " + exc);
  }
  if(lens->mRefCount <= 0) {
    // Perhaps should delete it?
    ZeroRefCount(lens);
  }

  if(mir->HasResultReq()) {
    TBuffer b(TBuffer::kWrite);
    b << lens->mSaturnID;
    mSaturn->ShootMIRResult(b);
  }

  return lens->mSaturnID;
}

/**************************************************************************/
// Lens deletion
/**************************************************************************/

void ZQueen::PutLensToPurgatory(ZGlass* lens)
{
  // First step in the process of a lens removal.
  // Stops all detached threads via Saturn::Freeze().
  // Removes all references to a lens and sends Rays with RQN_death.
  // Then emits MIR to PutLensToVoid(lens).
  // Called in a *detached thread*.

  static const string _eh("ZQueen::PutLensToPurgatory ");

  assert_MIR_presence(_eh, ZGlass::MC_IsFlare);

  if(lens->mQueen != this)
    throw(_eh + "lens " + lens->Identify() + " is not my subject.");

  QueenIDMap_i map_it;

  { // Check lens state consistency.
    GMutexHolder refcnt_lck(mSubjectRefCntMutex);

    map_it = mIDMap.find(lens->mSaturnID);
    if(map_it == mIDMap.end())
      throw(_eh + "lens " + lens->Identify() + " not found in my subject list.");
    if(map_it->second->mState != LensDetails::LS_Alive)
      throw(_eh + "lens " + lens->Identify() + " has already been put to death-bed.");
  }

  { // Stop threads.
    mSaturn->Freeze(lens);
  }

  GMutexHolder rule_lck(mSaturn->mRulingLock);
  GLensWriteHolder qwr_lck(this);

  {
    ISdebug(2, _eh + "blocking new refs to " + lens->Identify() + ".");
    GMutexHolder refcnt_lck(mSubjectRefCntMutex);
    if(mKing->GetLightType() == ZKing::LT_Moon) {
      lens->SetAcceptRefs(false);
      lens->mGlassBits |= ZGlassBits::kDying;
    }
  }

  {
    ISdebug(2, _eh + "removing all refs from " + lens->Identify() + ".");
    GLensReadHolder rdlck(lens);
    lens->ClearAllReferences();
  }

  // printf("%s %s RC=%d, MRC=%d, SRC=%d, FRC=%d\n", _eh.c_str(), lens->Identify().c_str(),
  //        lens->mRefCount, lens->mMoonRefCount, lens->mSunRefCount, lens->mFireRefCount);

  { // Remove all references to lens.
    GMutexHolder refcnt_lck(mSubjectRefCntMutex);

    ISdebug(2, _eh + "removing all refs to " + lens->Identify() + ".");
    set<ZGlass*> done_set;
    while( ! lens->mReverseRefs.empty() ) {
      ZGlass* ref = lens->mReverseRefs.front();
      if(done_set.find(ref) != done_set.end()) {
	ISwarn(_eh + "remaining reference from " + ref->Identify() +
	       " to " + lens->Identify() + ".");
	lens->mReverseRefs.pop_front();
	continue;
      }
      GLensWriteHolder reflens_wrlck();
      ref->remove_references_to(lens); // calls DecRefCount()
    }
    if(lens->mRefCount != 0) {
      ISerr(_eh + "ref-count not zero after enforced reverse refs removal.");
      ISerr(GForm("%s %s  RC=%d, MRC=%d, SRC=%d, FRC=%d\n",
		  _eh.c_str(), lens->Identify().c_str(),
		  lens->mRefCount, lens->mMoonRefCount,
		  lens->mSunRefCount, lens->mFireRefCount));
    }
  }

  // Emit the death Ray.
  auto_ptr<Ray> death_ray
    (Ray::PtrCtor(lens, RayNS::RQN_death, ++(lens->mTimeStamp), Ray::EB_StructuralChange));
  mSaturn->Shine(death_ray);

  map_it->second->mState = LensDetails::LS_Purged;
  mPurgatory.push_back(lens->mSaturnID);
  --mIDsUsed; ++mIDsPurged;

  SaturnInfo* si = mSaturn->GetSaturnInfo();
  GTime ref_time(GTime::I_Now);
  auto_ptr<ZMIR> void_mir( S_PutLensToVoid(lens->mSaturnID) );
  ref_time += 1000l*mPurgedMS;
  mSaturn->delayed_shoot_mir(void_mir, si, ref_time);

  Stamp(LibID(), ClassID());
}

void ZQueen::PutLensToVoid(ID_t lens_id)
{
  // Final step in lens removal.
  // Endarks the lens.
  // Clears up internal structures.
  // Deletes the lens.
  // Called in a *detached thread*.

  static const string _eh("ZQueen::PutLensToVoid ");

  assert_MIR_presence(_eh, ZGlass::MC_IsFlare);

  ZGlass* lens = mSaturn->DemangleID(lens_id);
  if(lens == 0) {
    // There is still a possibility that we are a moon and that
    // we connected between PutLensToPurgatory() and PutLensToVoid()
    // calls. Could check timing.
    if(mKing->GetLightType() == ZKing::LT_Moon) {
      GMutexHolder refcnt_lck(mSubjectRefCntMutex);
      QueenIDMap_i i = mIDMap.find(lens_id);
      if(i == mIDMap.end()) {
	ISerr(_eh + GForm("LensDetails for id=%d (expected as purged) not found in queen's ID map.", lens_id));
	return;
      }
      assert(i->second->mLens == 0);
      assert(i->second->mState == LensDetails::LS_Purged);
      i->second->mState = LensDetails::LS_Dead;
      i->second->mDeletionTime.SetNow();
      return;
    } else {
      throw(_eh + GForm("lens id=%d not elightened.", lens_id));
    }
  }

  if(lens->mQueen != this)
    throw(_eh + "lens " + lens->Identify() + " is not my subject.");

  // printf("%s %s RC=%d, MRC=%d, SRC=%d, FRC=%d\n", _eh.c_str(), lens->Identify().c_str(),
  //	    lens->mRefCount, lens->mMoonRefCount, lens->mSunRefCount, lens->mFireRefCount);

  GMutexHolder ruling_lck(mSaturn->mRulingLock);
  GLensWriteHolder queenwrite_lck(this);
  GMutexHolder refcnt_lck(mSubjectRefCntMutex);

  QueenIDMap_i i = mIDMap.find(lens_id);
  if(i == mIDMap.end()) {
    throw(_eh + "LensDetails of " + lens->Identify() + " not found in queen's ID map.");
  }
  if(i->second->mState != LensDetails::LS_Purged) {
    ISwarn(_eh + "lens has not been put to purged state ... expect trouble.");
  }
  if(i->second->mLens != lens) {
    ISwarn(_eh + "Saturn's and Queen's lens pointers do not mach ... expect trouble.");
  }
  ISdebug(2, _eh + "endarking lens " + lens->Identify() + ".");
  {
    GMutex::Lock_e lockp = lens->mReadMutex.TryLock();
    if(lockp != GMutex::ok) {
      ISwarn(_eh + GForm("failed acquiring ReadLock on '%s'. Proceeding anyway.",
			 lens->GetName()));
    }
    try {
      mSaturn->Endark(lens);
    }
    catch(string exc) {
      if(lockp == GMutex::ok) lens->mReadMutex.Unlock();
      throw(_eh + "endarkenment failed: " + exc + " Doing nothing.");
    }
    if(lockp == GMutex::ok) lens->mReadMutex.Unlock();
  }
  // !!!! Check for remaing images on eyes ... check improper, no locking.
  if(lens->mEyeRefCount != 0) {
    printf("SAFR ... eye refcount is %d!\n", lens->mEyeRefCount);
  }
  
  ISdebug(2, _eh + GForm("final removal of lens '%s'[%d].", lens->GetName(), i->first));
  i->second->mLens = 0;
  i->second->mState = LensDetails::LS_Dead;
  i->second->mDeletionTime.SetNow();
  delete lens;
  
  if(mKing->GetLightType() != ZKing::LT_Moon) {
    release_purgatory(0);
  }
  Stamp(LibID(), ClassID());
}

void ZQueen::RemoveLens(ZGlass* lens)
{
  // Initiates removal of lens. This is the method that should be called
  // by user.
  // If called as Flare broadcasting is suppressed.

  static const string _eh("ZQueen::RemoveLens ");

  if(mKing->GetLightType() == ZKing::LT_Moon)
    throw(_eh + "can not be called at a moon.");

  if(lens->mQueen != this)
    throw(_eh + "lens " + lens->Identify() + " is not my subject.");

  if(lens == this)
    throw(_eh + "attempt to delete ZQueen " + Identify() + ".");

  if(lens->mGlassBits & ZGlassBits::kDying)
    throw(_eh + "lens already dying.");

  ZMIR* mir = GThread::get_mir();
  if(mir && mir->IsFlare())
    mir->SuppressFlareBroadcast = true;

  SubjectRefCntLock();
  lens->SetAcceptRefs(false);
  lens->mGlassBits |= ZGlassBits::kDying;
  SubjectRefCntUnlock();

  SaturnInfo* si = mSaturn->GetSaturnInfo();
  auto_ptr<ZMIR> purg_mir( S_PutLensToPurgatory(lens) );
  mSaturn->ShootMIR(purg_mir);
}

/**************************************************************************/
// ZeroRefCount and management of Orphans
/**************************************************************************/

void ZQueen::ZeroRefCount(ZGlass* lens)
{
  // Called by ZGlass upon hitting refcount of 0.
  // mSubjectRefCntMutex is (and should be) locked prior to call.

  static const string _eh("ZQueen::ZeroRefCount ");

  if(mKing->GetLightType() == ZKing::LT_Moon) return;
  if(lens->mGlassBits & ZGlassBits::kDying)   return;

  switch(mZeroRCPolicy) {
  case ZRCP_Delete: {
    RemoveLens(lens);
    break;
  }    
  case ZRCP_ToOrphanage: {
    mOrphans->Add(lens);
    break;
  }
  case ZRCP_Ignore:
    break;
  }
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
// SetMandatory
/**************************************************************************/
/*
  // This must be heavily changed ... must emit eunuchs etc.

  void ZQueen::SetMandatory(Bool_t mandp)
  {
  static string _eh("ZQueen::SetMandatory");
  ZMIR* mir = assert_MIR_presence(_eh);

  if(mandp == bMandatory) return;

  if(mandp) {
  if(bRuling) {
  // mSaturn->BroadcastMIR(*mir, mReflectors);
      
  } else {

  }
  bMandatory = true;
  } else {
  bMandatory = false;
  }
  Stamp(LibID(), ClassID());
  }
*/

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

  release_purgatory(0);
  ZComet* c = MakeComet();
  c->Streamer(buf);
  delete c;
}

void ZQueen::InvokeReflection(TBuffer& buf)
{
  // Virtual. Invokes reflection created by a higher Saturn and thus
  // begins mirroring the object-space.
  // For ZQueen this involves unstreaming of ZComet and calling UnfoldFrom().

  static const string _eh("ZQueen::InvokeReflection ");

  ZComet c; c.Streamer(buf);
  if(c.GetType() != ZComet::CT_Queen) throw(_eh + "wrong comet type.");

  c.AssignQueen(this);
  c.SetExtDemangler(this);
  c.bVerbose = false;

  { // remove and delete comet's Deps, wipe it from comet's queen
    ID_t cdep_id = (ID_t)c.mQueen->GetDeps();
    assert(cdep_id == mDeps->GetSaturnID());
    ZGlass* cdep = c.DemangleID(cdep_id);
    delete cdep;
    c.mIDMap.erase(cdep_id);
    c.mQueen->mDeps = 0;
    // And remove the queen itself from the list
    assert(mSaturnID == c.mQueen->mSaturnID);
    c.mIDMap.erase(mSaturnID);
  }
  { // Rebuild the comet queen itself, assign stuff to *this* and delete it
    c.mQueen->RebuildLinkRefs(&c);
    c.mQueen->RebuildListRefs(&c);
    embrace_of_life(c);
    delete c.mQueen;
  }

  c.RebuildGraph();

  UnfoldFrom(c);
  // Now there is too much stuff in UnfoldFrom ... besides, SunQueen
  // has some link-related junk, too.
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

  for(QueenIDMap_i i=mIDMap.begin(); i!=mIDMap.end(); ++i) {
    if(i->second->mState == LensDetails::LS_Alive)
      comet->AddGlass(i->second->mLens);
  }

  return comet;
}

void ZQueen::AdoptComet(ZList* top_dest, ZList* orphan_dest, ZComet* comet)
{
  // Adopts a Comet of type ST_CometBag.
  // External references of a Comet are ignored.

  if(top_dest == 0)       top_dest = this;
  if(orphan_dest == 0) orphan_dest = mOrphans;

  // Should assert that top_dest & orphan_dest are my subjects.
  // Or at least that their queen depends on me.
  //    But then should do further authorization!

  comet->RebuildGraph();
  WriteLock();  
  UInt_t free = mIDSpan - (mIDMap.size() + mPurgatory.size());
  UInt_t need = comet->mIDMap.size();
  if(free >= need) {
    for(mID2pZGlass_i i=comet->mIDMap.begin(); i!=comet->mIDMap.end(); ++i)
      CheckIn(i->second);
    for(lpZGlass_i i=comet->mTopLevels.begin(); i!=comet->mTopLevels.end(); ++i)
      top_dest->Add(*i);
    for(lpZGlass_i i=comet->mOrphans.begin(); i!=comet->mOrphans.end(); ++i)
      orphan_dest->Add(*i);
  } else {
    WriteUnlock();
    throw(string("ZQueen::Adopt(comet=") + comet->GetName() + ") ID shortage");
  }
  WriteUnlock();
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

  WriteLock();
  { // Reflect all lenses
    mID2pZGlass_i      i = comet.mIDMap.begin();
    QueenIDMap_i ins_pos = mIDMap.begin();
    while(i != comet.mIDMap.end()) {
      i->second->SetStamps(1);
      mSaturn->Reflect(i->second);
      ins_pos = mIDMap.insert
	( ins_pos, pair<ID_t, LensDetails*>
	  (i->first, produce_lens_details(i->first, i->second))
	);
      ++i;
    }
  }
  { // Create dummy entries for lenses in purgatory.
    for(lID_i i=mPurgatory.begin(); i!=mPurgatory.end(); ++i) {
      LensDetails* ls = produce_lens_details(*i, 0);
      ls->mState = LensDetails::LS_Purged;
      mIDMap.insert(pair<ID_t, LensDetails*>(*i, ls));
    }
  }

  bRuling = true;

  // Call AdUnfoldment for all lenses
  for(mID2pZGlass_i i=comet.mIDMap.begin(); i!=comet.mIDMap.end(); ++i) {
    i->second->AdUnfoldment();
  }

  Stamp();
  StampLink();
  StampListRebuild();

  WriteUnlock();
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
// Maintenance of non-ruling queen reflections
/**************************************************************************/

void ZQueen::BroadcastMIRToNonRulingReflections(ZMIR& mir)
{
  static const string _eh("ZQueen::BroadcastMIRToNonRulingReflections ");

  // This is way sub-optimal ... need iterator based broadcast MIR and
  // set representation of reflectors and moons (instead of list).

  lpSaturnInfo_t foo; mSaturn->CopyMoons(foo);
  for(lpSaturnInfo_i i=mReflectors.begin(); i!=mReflectors.end(); ++i) {
    foo.remove(*i);
  }
  mSaturn->BroadcastMIR(mir, foo);
}

void ZQueen::BasicQueenChange(ZMIR& mir)
{
  static const string _eh("ZQueen::BasicQueenChange ");

  if(mir.Alpha != this)  throw(_eh + "alpha is not *this* queen.");
  if(mir.HasRecipient()) throw(_eh + "MIR is a beam, should be flare.");
  if(mir.Lid != ZGlass::LibID() || mir.Cid != ZGlass::ClassID())
    throw(_eh + "FID does not correspond to ZGlass.");
  if(mir.Mid != ZGlass::Mid_SetName() && mir.Mid != ZGlass::Mid_SetTitle())
    throw(_eh + "MID does not correspond to SetName or SetTitle.");

  BroadcastMIRToNonRulingReflections(mir);
}

/**************************************************************************/
// Stamping
/**************************************************************************/

void ZQueen::EmitRay(auto_ptr<Ray>& ray)
{
  mSaturn->Shine(ray);
}

/**************************************************************************/
// tmp
/**************************************************************************/

void ZQueen::ListAll()
{
  // attempt at looking at all subjects based on ids
  // else might be forced into having local hash.

  for(QueenIDMap_i i=mIDMap.begin(); i!=mIDMap.end(); ++i) {
    ZGlass* l = i->second->mLens;
    if(l != 0) {
      cout <<"ID="<< i->first <<": "<< l->GetName() <<"["<< l->ZlassName() <<"]\n";
    } else {
      cout <<"ID="<< i->first <<": <reserved>\n";
    }
  }
}

/**************************************************************************/

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//________________________________________________________________________
// ZKing
//
// A King controls top level chunks of ID space. A sun-space of each Saturn
// is ruled over by a King.
// Each fire-space of each Saturn is also ruled by a King. Flag
// bFireKing tells if the king is a ruler of fire.
//
// BlessMIR() method is much less restrictive than the Queen version
// as Kings provide services for managing dependencies and mirroring
// of queens. Visibility of arguments is the sole concern of this
// method. Further checking is (and should be) provided in each
// respective method.
//________________________________________________________________________

#include "ZKing.h"
#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>
#include <Ephra/Saturn.h>
#include <Gled/GledNS.h>

#include <Stones/ZMIR.h>
#include <Stones/ZComet.h>



ClassImp(ZKing)

/**************************************************************************/

void ZKing::_init()
{
  mSaturnInfo = 0;
  mLightType = LT_Undef;
  bFireKing = false;
  mMapNoneTo = ZMirFilter::R_Allow;
}

/**************************************************************************/

void ZKing::BlessMIR(ZMIR& mir) throw(string)
{
  // Should check that non-zero beta/gamma belong to this or a higher
  // object-space.

  static string _eh("ZKing::BlessMIR() ");

  // Authorization

  // Allow everything if UseAuth is false
  if(mSaturn->GetSaturnInfo()->GetUseAuth() == false) {
    return;
  }

  // Always allow SunAbsolute access
  if(mir.Caller->HasIdentity(mSaturn->mSunInfo->GetPrimaryIdentity())) {
    return;
  }

  UChar_t result = (mGuard != 0) ? mGuard->FilterMIR(mir) : ZMirFilter::R_None;
  if( result == ZMirFilter::R_Deny ||
      (result == ZMirFilter::R_None && mMapNoneTo == ZMirFilter::R_Deny))
    {
      throw(_eh + "access denied");
    }
}

/**************************************************************************/

void ZKing::Enthrone(ZQueen* queen)
{
  ID_t w = queen->GetIDSpan();
  if(mMaxID==mMaxUsedID || mMaxID - mMaxUsedID < queen->mIDSpan) {
    assert(0);    
  }
  queen->mKing   = this;
  queen->mSaturn = mSaturn;
  queen->mMinID = mMaxUsedID + 1;
  mMaxUsedID += queen->mIDSpan;
  queen->mMaxID = mMaxUsedID;
  queen->mMaxUsedID = queen->mMinID - 1;
  queen->Bootstrap();
  Add(queen);

  if(bFireKing) {
    queen->SetAuthMode(ZQueen::AM_None);
  }

  // The first queen is mQueen. Used for emitting Rays.
  if(mQueen == 0)
    mQueen = queen;

  // should broadmoon via streaming ... or whatever.
  // perhaps solve at sun level. More like a helper for whore queens.
}

void ZKing::StarToQueen(ZComet* comet, ID_t span)
{
  // In principle, loading of queen with wider id-span than needed.
  // Perhaps could be even useful.
}

/**************************************************************************/

ZComet* ZKing::MakeComet()
{
  // Produces Comet of type ST_King.
  // Contains the King, all its Queens and their Dependencies.
  // Used for producing snapshot of a King-space to be sent to a Moon.

  ZComet* comet = new ZComet(GetName(), GForm("Comet[King] of %s", GetName()));
  comet->mType = ZComet::CT_King;
  comet->mKing = this;

  mExecMutex.Lock();
  comet->AddGlass(this);
  lpZGlass_t queens; Copy(queens);
  for(lpZGlass_i i=queens.begin(); i!=queens.end(); ++i) {
    ZQueen* q = dynamic_cast<ZQueen*>(*i);
    assert(q!=0);
    comet->AddGlass(q);
    comet->AddGlass(q->GetDeps());
  }
  mExecMutex.Unlock();

  return comet;
}

/**************************************************************************/
// Mirroring of Queens
/**************************************************************************/

void ZKing::ReflectQueen(ZQueen* queen_to_mirror)
{
  // Entry point for initiation of queen mirroring on user request.
  // Should be directed at FireKing, who performs the task
  // on behalf of Saturn.
  // (would be more proper to have MirrorQueen or MirrorPrincess in fire-space)

  static string _eh("ZKing::ReflectQueen() ");

  // Should assert queen exists etc ...

  if(!bFireKing) throw(_eh + "should be directed at FireKing");
  ZKing* boss = queen_to_mirror->GetKing();
  if(boss->GetMinID() > mSaturn->GetKing()->GetMinID()) {
    throw(_eh + "queen should be in above object-space");
  }

  auto_ptr<ZMIR> mir(boss->S_reflect_queen(queen_to_mirror, mSaturn->GetSaturnInfo()));
  mir->SetRecipient(mSaturn->GetSaturnInfo());
  mSaturn->PostMIR(*mir);

  // assert queen in `above' space and not yet ruling, neither awaiting sceptre.
  // pass control to reflect_queen, which also checks dependencies and
  // generates appropriate beams for the master Saturn.
}

void ZKing::UnreflectQueen(ZQueen* queen_to_leave)
{
  // Ignore gloriously. Deps first.
}

/**************************************************************************/

void ZKing::reflect_queen(ZQueen* queen_to_mirror, SaturnInfo* moon)
{
  // Actual activation of a queen.
  // If moon == mSaturn->GetSaturnInfo() and queen is not activated,
  //    check deps, send beams, mark the queens.
  // If moon in my moons, then:
  //   if(have the queen active) push it along
  //   else forward the request and mark thingies (queen and the new reflector).

  static string _eh("ZKing::reflect_queen() ");

  if(mMir == 0) {
    throw(_eh + "should be called with mMir set");
  }

  if(moon == mSaturn->GetSaturnInfo()) {
    // Request came from self ... must beam it upwards, if necessary.

    if(queen_to_mirror->GetRuling()) {
      throw(_eh + "queen is already ruling");
    }
    if(queen_to_mirror->GetAwaitingSceptre()) {
      throw(_eh + "queen is awaiting sceptre");
    }

    // Here should check dependencies etc.

    mSaturn->RefQueenLoadCnd().Lock();
    mSaturn->SetQueenLoadNum(mSaturn->GetQueenLoadNum() + 1);
    mSaturn->RefQueenLoadCnd().Broadcast();
    mSaturn->RefQueenLoadCnd().Unlock();


    queen_to_mirror->SetAwaitingSceptre(true);
    // Request_mirror
    auto_ptr<ZMIR> mir(S_reflect_queen(queen_to_mirror, moon));
    mir->SetCaller(mSaturn->GetSaturnInfo());
    mir->SetRecipient(moon->GetMaster());
    mSaturn->PostMIR(*mir);

  } else {
    // Request should be coming from direct moon.
    // Must serve it or forward it.

    if(!mSaturn->IsMoon(moon)) {
      throw(_eh + "moon neither this Saturn nor its direct Moon");
    }

    if(queen_to_mirror->GetRuling()) {

      queen_to_mirror->mExecMutex.Lock();
      auto_ptr<ZMIR> mir(this->S_activate_queen(queen_to_mirror));
      mir->SetCaller(mSaturn->GetSaturnInfo());
      mir->SetRecipient(moon);
      queen_to_mirror->CreateReflection(*mir);
      queen_to_mirror->add_reflector(moon);
      queen_to_mirror->mExecMutex.Unlock();
      ISdebug(0, GForm("%s Sending queen %s to moon %s; length is=%d",
		       _eh.c_str(), queen_to_mirror->GetName(),
		       moon->GetName(), mir->Length()));
      mSaturn->PostMIR(*mir);
      moon->hQueens.insert(queen_to_mirror);

    } else if(queen_to_mirror->GetAwaitingSceptre()) {

      queen_to_mirror->add_reflector(moon);

    } else {

      // Here should check dependencies etc.
      queen_to_mirror->SetAwaitingSceptre(true);
      queen_to_mirror->add_reflector(moon);
      // Request_mirror
      auto_ptr<ZMIR> mir(S_reflect_queen(queen_to_mirror,
					 mSaturn->GetSaturnInfo()));
      mir->SetCaller(mSaturn->GetSaturnInfo());
      mir->SetRecipient(mSaturn->GetSaturnInfo()->GetMaster());
      mSaturn->PostMIR(*mir);

    }

  }
}

void ZKing::activate_queen(ZQueen* queen)
{
  // A response from upper Saturn, containing whatever was attached
  // by ZQueen::CreateReflection().

  // assert queen awaiting sceptre.
  // Invoke it upon reflection.
  // Check, if the queen has any aspiring reflectors ... forward the beam to them

  static string _eh("ZKing::activate_queen() ");

  if(!queen->GetAwaitingSceptre()) {
    throw(_eh + "queen " + queen->GetName() + " is NOT awaiting sceptre");
  }

  queen->InvokeReflection(*mMir);
  queen->SetAwaitingSceptre(false);

  ISmess(GForm("%s queen '%s' arrived for king '%s'",
	       _eh.c_str(), queen->GetName(), GetName()));

  mSaturn->RefQueenLoadCnd().Lock();
  mSaturn->SetQueenLoadNum(mSaturn->GetQueenLoadNum() - 1);
  mSaturn->RefQueenLoadCnd().Broadcast();
  mSaturn->RefQueenLoadCnd().Unlock();

  // Broadcast beams
  if(!queen->mReflectors.empty()) {
    mSaturn->BroadcastBeamMIR(*mMir, queen->mReflectors);
    for(lpSaturnInfo_i r=queen->mReflectors.begin(); r!=queen->mReflectors.end(); ++r) {
      (*r)->hQueens.insert(queen);
    }
  }
}

void ZKing::unreflect_queen(ZQueen* queen_to_leave, SaturnInfo* moon)
{
  // Ignore gloriously. Deps first.
}


/**************************************************************************/

#include "ZKing.c7"

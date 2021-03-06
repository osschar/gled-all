// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//________________________________________________________________________
// ZKing
//
// A King controls top level chunks of ID space. A sun-space of each Saturn
// is ruled over by a King.
// King creates a dummy queen (set to mQueen) for proper ref-counting.
// Fire-space of each Saturn is ruled by a ZFireKing.
//
// BlessMIR() method is much less restrictive than the Queen version
// as Kings provide services for managing dependencies and mirroring
// of queens. Visibility of arguments is the sole concern of this
// method. Further checking is (and should be) provided in each
// respective method.
//________________________________________________________________________

#include "ZKing.h"
#include <Glasses/ZQueen.h>
#include "ZKing.c7"
#include <Glasses/ZEunuch.h>
#include <Glasses/SaturnInfo.h>
#include <Stones/ZComet.h>

ClassImp(ZKing);

/**************************************************************************/

void ZKing::_init()
{
  mSaturnInfo = 0;
  mLightType = LT_Undef;
  mMapNoneTo = ZMirFilter::R_Allow;
}

/**************************************************************************/

ZKing::~ZKing()
{
  delete mQueen;
}

/**************************************************************************/

void ZKing::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  // Create a dummy queen for reference counting and emitting Rays.
  mQueen = new ZQueen(GForm("Concubine of %s", GetName()));
  mQueen->mSaturn = mSaturn;
  mQueen->mZeroRCPolicy = ZQueen::ZRCP_Ignore;
  mQueen->mKing = this;
}

/**************************************************************************/

void ZKing::BlessMIR(ZMIR& mir)
{
  // Performs dependenciy check of context arguments and access
  // authorization.

  static TString _eh("ZKing::BlessMIR ");

  // Dependency check
  // Mild version ... just assert args in moon or sun space.
  // Further could restrict args to queens + lenses of mandatory queens.
  if(mir.fBeta) {
    if(mir.fBetaID > mMaxID) {
      throw(_eh + GForm("beta '%s', id=%d: dependency check failed.",
			mir.fBeta->GetName(), mir.fBetaID));
    }
  }
  if(mir.fGamma) {
    if(mir.fGammaID > mMaxID) {
      throw(_eh + GForm("gamma '%s', id=%d: dependency check failed.",
			    mir.fGamma->GetName(), mir.fGammaID));
    }
  }

  // Authorization

  // Allow everything if UseAuth is false
  if(mSaturn->GetSaturnInfo()->GetUseAuth() == false) {
    return;
  }

  // Always allow SunAbsolute access
  if(mir.fCaller->HasIdentity(mSaturn->mSunInfo->GetPrimaryIdentity())) {
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
  ID_t id_span = queen->GetIDSpan();
  if(mMaxID==mMaxUsedID || mMaxID - mMaxUsedID < id_span) {
    assert(0);
  }
  queen->mKing   = this;
  queen->mSaturn = mSaturn;
  queen->mMinID = mMaxUsedID + 1;
  mMaxUsedID += id_span;
  queen->mMaxID = mMaxUsedID;
  queen->bootstrap();
  Add(queen);

  // !!!! should broadcast to all moons
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

  ReadLock();
  comet->AddGlass(this);
  lpZGlass_t queens; CopyList(queens);
  for(lpZGlass_i i=queens.begin(); i!=queens.end(); ++i) {
    ZQueen* q = dynamic_cast<ZQueen*>(*i);
    assert(q!=0);
    comet->AddGlass(q);
    comet->AddGlass(q->GetDeps());
  }
  ReadUnlock();

  return comet;
}

/**************************************************************************/
// Mirroring of Queens
/**************************************************************************/

void ZKing::reflect_queen(ZQueen* queen_to_mirror, SaturnInfo* moon)
{
  // Actual activation of a queen.
  // If moon == mSaturn->GetSaturnInfo() and queen is not activated,
  //    check deps, send beams, mark the queens.
  // If moon in my moons, then:
  //   if(have the queen active) push it along
  //   else forward the request and mark thingies (queen and the new reflector).

  static TString _eh("ZKing::reflect_queen() ");

  assert_MIR_presence(_eh);

  if(moon == mSaturn->GetSaturnInfo()) {
    // Request came from self ... must beam it upwards, if necessary.

    if(queen_to_mirror->GetRuling()) {
      throw(_eh + "queen is already ruling");
    }
    if(queen_to_mirror->GetAwaitingSceptre()) {
      throw(_eh + "queen is awaiting sceptre");
    }

    // Here should check dependencies etc.
    // or perhaps later prior to streaming

    mSaturn->RefQueenLoadCnd().Lock();
    mSaturn->SetQueenLoadNum(mSaturn->GetQueenLoadNum() + 1);
    mSaturn->RefQueenLoadCnd().Broadcast();
    mSaturn->RefQueenLoadCnd().Unlock();


    queen_to_mirror->SetAwaitingSceptre(true);
    // Request_mirror
    auto_ptr<ZMIR> mir(S_reflect_queen(queen_to_mirror, moon));
    mir->SetCaller(mSaturn->GetSaturnInfo());
    mir->SetRecipient(moon->GetMaster());
    mSaturn->PostMIR(mir);

  } else {
    // Request should be coming from direct moon.
    // Must serve it or forward it.

    if(!mSaturn->IsMoon(moon)) {
      throw(_eh + "moon neither this Saturn nor its direct Moon");
    }

    if(queen_to_mirror->GetRuling()) {

      queen_to_mirror->WriteLock();
      auto_ptr<ZMIR> mir(this->S_activate_queen(queen_to_mirror));
      mir->SetCaller(mSaturn->GetSaturnInfo());
      mir->SetRecipient(moon);
      queen_to_mirror->CreateReflection(*mir);
      queen_to_mirror->add_reflector(moon);
      queen_to_mirror->WriteUnlock();
      ISdebug(0, GForm("%s Sending queen '%s' to moon '%s'; length=%d",
		       _eh.Data(), queen_to_mirror->GetName(),
		       moon->GetName(), mir->Length()));
      mSaturn->PostMIR(mir);
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
      mSaturn->PostMIR(mir);

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

  static TString _eh("ZKing::activate_queen() ");

  ZMIR* mir = assert_MIR_presence(_eh);

  if(!queen->GetAwaitingSceptre()) {
    throw(_eh + "queen " + queen->GetName() + " is NOT awaiting sceptre");
  }

  queen->InvokeReflection(*mir);
  queen->SetAwaitingSceptre(false);

  ISmess(GForm("%s queen '%s' arrived for king '%s'",
	       _eh.Data(), queen->GetName(), GetName()));

  mSaturn->RefQueenLoadCnd().Lock();
  mSaturn->SetQueenLoadNum(mSaturn->GetQueenLoadNum() - 1);
  mSaturn->RefQueenLoadCnd().Broadcast();
  mSaturn->RefQueenLoadCnd().Unlock();

  // Broadcast beams
  if(!queen->mReflectors.empty()) {
    mSaturn->BroadcastBeamMIR(*mir, queen->mReflectors);
    for(ZQueen::lpSaturnInfo_i r=queen->mReflectors.begin(); r!=queen->mReflectors.end(); ++r) {
      (*r)->hQueens.insert(queen);
    }
  }
}

void ZKing::unreflect_queen(ZQueen* queen_to_leave, SaturnInfo* moon)
{
  // Ignore gloriously. Deps first.
}

void ZKing::receive_eunuch()
{
  // Receives an eunuch.

  static TString _eh("ZKing::receive_eunuch ");
  ZMIR* mir = assert_MIR_presence(_eh, ZGlass::MC_IsBeam);

  ZEunuch* e = GledNS::StreamLensByGlass<ZEunuch*>(*mir);
  if(e == 0)
    throw(_eh + "MIR not followed by an eunuch.");


}


/**************************************************************************/

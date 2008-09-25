// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


//______________________________________________________________________
// ZSunQueen
//
// The first queen of the Sun Absolute, holding information about all 
// Saturns/Eyes connected to the Sun. These structures will also be used
// for authentication of connecting eyes/moons and to hold their certificates.


#include "ZSunQueen.h"
#include <Glasses/EyeInfo.h>
#include <Stones/ZMIR.h>
#include <Stones/ZComet.h>
#include <Ephra/Saturn.h>
#include <Gled/GledNS.h>
#include <TMessage.h>

ClassImp(ZSunQueen)

void ZSunQueen::_init()
{
  mSunInfo = 0;
}

/**************************************************************************/

void ZSunQueen::UnfoldFrom(ZComet& comet)
{
  ZSunQueen* sq = dynamic_cast<ZSunQueen*>(comet.mQueen);
  assert(comet.mType == ZComet::CT_Queen && sq != 0);

  mSunInfo = sq->mSunInfo;
  ZQueen::UnfoldFrom(comet);
}

/**************************************************************************/

void ZSunQueen::IncarnateMoon(SaturnInfo* parent)
{
  assert(mMir != 0);
  SaturnInfo* mi = dynamic_cast<SaturnInfo*>
    (GledNS::StreamGlass(*mMir->Message));
  assert(mi!=0);
  // !!!! if mSaturn->GetSaturnInfo() == mSunInfo should:
  //   check/modify the saturn_info and recreate MIR as flare.
  //   upon failiure send beam to caller, canceling the request.
  //   Might as well change the Caller to self.
  // For now ... just accept everything as is.
  CheckIn(mi);
  mi->create_lists();
  
  if(mSaturn->GetSunAbsolute()) {
    mMir->ClearRecipient();
    mMir->SetCaller(mSunInfo);
    mSaturn->BroadcastMIR(*mMir, mSaturn->mMoons);
  }

  parent->GetMoons()->Add(mi);
  mi->SetMaster(parent);
  if(mSaturn->GetSaturnInfo() == parent) {
    mi->hSocket = mSaturn->mPendingSocket;
    mSaturn->new_moon_ready(mi);
  }
}

void ZSunQueen::IncarnateEye(SaturnInfo* parent)
{
  assert(mMir != 0);

  EyeInfo* ei = dynamic_cast<EyeInfo*>
    (GledNS::StreamGlass(*mMir->Message));
  assert(ei!=0);
  // !!!! if  mSaturn->GetSunAbsolute() should:
  //   check/modify the saturn_info and recreate MIR as flare.
  //   upon failiure send beam to caller, canceling the request.
  // For now ... just accept everything as is.
  CheckIn(ei);
  
  if(mSaturn->GetSunAbsolute()) {
    mMir->ClearRecipient();
    mMir->SetCaller(mSunInfo);
    mSaturn->BroadcastMIR(*mMir, mSaturn->mMoons);
  }

  parent->GetEyes()->Add(ei);
  ei->SetMaster(parent);
  if(mSaturn->GetSaturnInfo() == parent) {
    ei->hSocket = mSaturn->mPendingSocket;
    mSaturn->new_eye_ready(ei);
  }
}

void ZSunQueen::CremateMoon(SaturnInfo* moon)
{
  // Removes moon (and all its satellites) from celestial bodies.
  // If sent as MT_Beam interprets it as request to shut down
  // connection to the moon with notification sent to SunQueen.

  assert(mMir != 0);

  if(moon->GetMaster() == mSaturn->GetSaturnInfo()) {
    if(moon->hSocket != 0) {
      ISmess(GForm("ZSunQueen::CremateMoon socket still present ... closing it"));
      if(mMir->Message->What() == GledNS::MT_Beam) {
	mSaturn->wipe_moon(moon, true);
	return;
      } else {
	mSaturn->wipe_moon(moon, false);
      }
    }
  }

  if(mSaturn->GetSunAbsolute()) {
    mMir->ClearRecipient();
    mMir->SetCaller(mSunInfo);
    mSaturn->BroadcastMIR(*mMir, mSaturn->mMoons);
  }

  mOrphans->Add(moon);
  moon->GetMaster()->GetMoons()->Remove(moon);
  moon->SetMaster((SaturnInfo*)0); moon->hRoute = 0;
}

void ZSunQueen::CremateEye(EyeInfo* eye)
{

  assert(mMir != 0);

  if(eye->GetMaster() == mSaturn->GetSaturnInfo()) {
    if(eye->hSocket != 0) {
      ISmess(GForm("ZSunQueen::CremateEye socket still present ... closing it"));
      if(mMir->Message->What() == GledNS::MT_Beam) {
	mSaturn->wipe_eye(eye, true);
	return;
      } else {
	mSaturn->wipe_eye(eye, false);
      }
    }
  }

  if(mSaturn->GetSunAbsolute()) {
    mMir->ClearRecipient();
    mMir->SetCaller(mSunInfo);
    mSaturn->BroadcastMIR(*mMir, mSaturn->mMoons);
  }

  mOrphans->Add(eye);
  eye->GetMaster()->GetEyes()->Remove(eye);
  eye->SetMaster((SaturnInfo*)0);
}

/**************************************************************************/

#include "ZSunQueen.c7"

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SaturnInfo.h"
#include <Ephra/Saturn.h>
#include <Glasses/ZQueen.h>
#include <Glasses/EyeInfo.h>
#include <Gled/GledNS.h>

#include <TSocket.h>

#include <memory>

ClassImp(SaturnInfo)

SaturnInfo::SaturnInfo(const Text_t* n, const Text_t* t) : ZGlass(n,t)
{
  mHostName = "";   mServerPort = GLED_DEF_PORT;
  mMasterName = ""; mMasterPort = GLED_DEF_PORT;

  mCPU_Model = "<unknown>"; mCPU_Freq = mCPU_Num = 0;
  mMemory = mSwap = mMFree = mSFree = 0;
  mLAvg1 = mLAvg5 = mLAvg15 = 0;
  mCU_Total = mCU_User = mCU_Nice = mCU_Sys = 0;

  mSunSpaceSize = mKingID = mFireKingID = 0;

  mMaster = 0; mMoons = 0; mEyes = 0;

  hSocket = 0; hRoute = 0;
}

/**************************************************************************/

void SaturnInfo::create_lists()
{
  ZHashList* l;
  l = new ZHashList(GForm("Moons of %s", mName.Data()));
  mQueen->CheckIn(l); SetMoons(l);
  l = new ZHashList(GForm("Eyes of %s", mName.Data()));
  mQueen->CheckIn(l); SetEyes(l);
}

/**************************************************************************/

void SaturnInfo::AddMoon(SaturnInfo* moon)
{
  // blabla ...
  // moon->create_lists
}

void SaturnInfo::AddEye(EyeInfo* eye)
{

}

/**************************************************************************/

#include "SaturnInfo.c7"

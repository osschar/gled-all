// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// NestInfo
//
//

#include "NestInfo.h"
#include "NestInfo.c7"

#include <Ephra/Saturn.h>
#include <Glasses/ZGod.h>
#include <Glasses/ZQueen.h>

ClassImp(NestInfo)

/**************************************************************************/

void NestInfo::_init()
{
  // Override from SubShellInfo:
  mCtorLibset = "GledCore";
  mCtorName   = "FTW_Nest";

  bShowSelf   = false;
  mMaxChildExp   = 1;

  mLeafLayout = LL_Ants;
}

/**************************************************************************/

void NestInfo::EnactLayout()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_user_1, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

void NestInfo::ImportKings()
{
  lpZGlass_t kings; mSaturn->GetGod()->Copy(kings);
  for(lpZGlass_i k=kings.begin(); k!=kings.end(); ++k)
    Add(*k);
}

/**************************************************************************/

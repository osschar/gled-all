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

const Text_t* NestInfo::sLayoutPath = "Etc/NestLayouts";

void NestInfo::_init()
{
  // Override from SubShellInfo:
  mCtorLibset = "GledCore";
  mCtorName   = "FTW_Nest";

  bShowSelf   = false;
  mMaxChildExp   = 1;

  mWName   = 30; mWAnt    = 0;
  mWIndent = 2;  mWSepBox = 1;

  mLayoutList = 0;
  mLeafLayout = LL_Ants;
}

/**************************************************************************/

void NestInfo::ImportLayout(ZGlass* src)
{
  ZList* lsrc = dynamic_cast<ZList*>(src);
  if(lsrc != 0) {
    lStr_t     parts;
    lpZGlass_t l; lsrc->Copy(l);
    for(lpZGlass_i i=l.begin(); i!=l.end(); ++i)
      parts.push_back((*i)->GetTitle());
    mLayout = GledNS::join_strings(" : ", parts);
    mLeafLayout = LL_Custom;
  } else {
    mLayout     = src->GetTitle();
    mLeafLayout = LL_Custom;
  }
  Stamp(FID());
  EmitLayoutRay();
}

/**************************************************************************/

void NestInfo::ImportKings()
{
  lpZGlass_t kings; mSaturn->GetGod()->Copy(kings);
  for(lpZGlass_i k=kings.begin(); k!=kings.end(); ++k)
    Add(*k);
}

/**************************************************************************/

void NestInfo::EmitLayoutRay()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_user_1, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

void NestInfo::EmitRewidthRay()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_rewidth, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// NestInfo
//
//

#include "NestInfo.h"
#include <Ephra/Saturn.h>
#include <Glasses/ZGod.h>
#include <Glasses/ZQueen.h>

ClassImp(NestInfo)

/**************************************************************************/

void NestInfo::_init()
{
  mPupils = 0;
  bSpawnPupils = true;
}

/**************************************************************************/

void NestInfo::AdEnlightenment()
{
  ZList::AdEnlightenment();
  if(mPupils == 0) {
    ZList* l = new ZList("PupilInfos", GForm("Pupils of %s", GetName()));
    mQueen->CheckIn(l);
    SetPupils(l);
  }
}

/**************************************************************************/

void NestInfo::ImportKings()
{
  lpZGlass_t kings; mSaturn->GetGod()->Copy(kings);
  for(lpZGlass_i k=kings.begin(); k!=kings.end(); ++k)
    Add(*k);
}

/**************************************************************************/

#include "NestInfo.c7"

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ShellInfo.h"
#include <Glasses/ZQueen.h>

ClassImp(ShellInfo)

/**************************************************************************/

void ShellInfo::_init()
{
  mNests = 0;
  mBeta = mGamma = 0;
  bSpawnNests = true;
}

/**************************************************************************/

void ShellInfo::AdEnlightenment()
{
  NestInfo::AdEnlightenment();
  if(mNests == 0) {
    ZList* l = new ZList("NestInfos", GForm("Nests of %s", GetName()));
    mQueen->CheckIn(l);
    SetNests(l);
  }
}

/**************************************************************************/

#include "ShellInfo.c7"

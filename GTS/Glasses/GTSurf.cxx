// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTSurf.h"

ClassImp(GTSurf)

void GTSurf::_Init()
{
  pSurf = 0; mScale = 1;
}

void GTSurf::Load()
{
  // missing dtor call
  delete pSurf;
  pSurf = GTS::Surface::Load(mFile);
  if(pSurf==0) {
    ISerr(GForm("GTSurf::Load gts_surface_new failed ..."));
    return;
  }
  Stamp();
}

void GTSurf::Invert()
{
  pSurf->Invert();
  Stamp();
}

#include "GTSurf.c7"

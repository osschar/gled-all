// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Lamp.h"

ClassImp(Lamp)

/**************************************************************************/

void Lamp::_init()
{
  mAmbient.rgba(0.05, 0.05, 0.05, 1);
  mDiffuse.rgba(0.5, 0.5, 0.5, 1);
  mSpecular.rgba(0.75, 0.75, 0.75, 1);
  mScale = 0;
  mSpotExp = 0; mSpotCutOff = 180;
  mConstAtt = 1; mLinAtt = mQuadAtt = 0;

  bDrawLamp = true; bOnIfOff = false; bOffIfOn = false;
}

/**************************************************************************/

#include "Lamp.c7"

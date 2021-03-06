// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Lamp.h"
#include "Lamp.c7"

ClassImp(Lamp);

/**************************************************************************/

void Lamp::_init()
{
  mAmbient.rgba(0.05, 0.05, 0.05, 1.0);
  mDiffuse.rgba(1, 1, 1, 1);
  mSpecular.rgba(0.2, 0.2, 0.2, 1.0);
  mLampScale = 0;
  mSpotExp = 0; mSpotCutOff = 180;
  mConstAtt = 1; mLinAtt = mQuadAtt = 0;

  bDrawLamp = true; bOnIfOff = false; bOffIfOn = false;
}

/**************************************************************************/

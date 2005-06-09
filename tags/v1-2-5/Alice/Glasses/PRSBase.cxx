// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PRSBase
//
//

#include "PRSBase.h"
#include "PRSBase.c7"

ClassImp(PRSBase)

/**************************************************************************/

Float_t PRSBase::sDefaultMagField = 0.4;

void PRSBase::_init()
{
  // *** Set all links to 0 ***/
  mMagField = sDefaultMagField;

  mVertexColor.rgba(1,0,0,1);
  mVertexSize = 5;
  mRnrP = false;
  mPColor.rgba(1,0,0,1);
  mPMinLen = 10;
  mPScale = 1;

  mTrackColor.rgba(1,1,0,1);
  mTrackWidth      = 1.2;
  mTrackStippleFac = 0;
  mTrackStipplePat = 0xCCCC;

  mMinP = 0.00005;
  mTheta = 90;
  mThetaOff = 90;
  mMaxR=600; 
  mMaxZ=550;
  mMaxOrbs=2;
  mDelta  = 0.1; //calculate step size depending of helix radius
  mMinAng = 45;

  //textures
  mTexture = 0;
  mTexFactor = 200;
  mTexVCoor = 0.2;
}

/**************************************************************************/


/**************************************************************************/

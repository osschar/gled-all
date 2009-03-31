// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// HitContainer
//
//

#include "HitContainer.h"
#include "HitContainer.c7"

ClassImp(HitContainer)

/**************************************************************************/

void HitContainer::_init()
{
  mNPoints = 0;
  mPointLabels = 0;
  mPoints      = 0;

  mSize = 0;
  mColor.rgba(0,0,1,1);
  // *** Set all links to 0 ***
}

/**************************************************************************/

HitContainer::~HitContainer()
{
  delete [] mPointLabels;
  delete [] mPoints;
}

/**************************************************************************/

void HitContainer::Reset(Int_t n_points)
{
  if(n_points != mNPoints) {
    delete [] mPointLabels;
    delete [] mPoints;
    mNPoints = n_points;
    if(mNPoints > 0) {
      mPointLabels = new   Int_t[mNPoints];
      mPoints      = new Float_t[3*mNPoints];
    } else {
      mPointLabels = 0;
      mPoints      = 0;
    }
  }
}

/**************************************************************************/

void HitContainer::SetPoint(Int_t i, Int_t label, Float_t* pos)
{
  mPointLabels[i] = label;
  Float_t* p = mPoints + 3*i;
  p[0] = pos[0]; p[1] = pos[1]; p[2] = pos[2];
}

void HitContainer::SetPoint(Int_t i, Int_t label,
			    Float_t x, Float_t y, Float_t z)
{
  mPointLabels[i] = label;
  Float_t* p = mPoints + 3*i;
  p[0] = x; p[1] = y; p[2] = z;
}

/**************************************************************************/
void  HitContainer::Print()
{
  for(Int_t i=0; i<mNPoints; i++){
    printf("Point%d V(%f,%f,%f)\n",mPointLabels[i], mPoints[i],mPoints[i+1],mPoints[i+2]);
  }
}

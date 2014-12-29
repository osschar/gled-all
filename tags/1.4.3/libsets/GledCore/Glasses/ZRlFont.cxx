// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZRlFont
//
//

#include "ZRlFont.h"
#include "ZRlFont.c7"
#include <Glasses/ZQueen.h>

#include "TSystem.h"

ClassImp(ZRlFont);

/**************************************************************************/

void ZRlFont::_init()
{
  mMode = FM_Texture;
  mFontFile = GForm("%s/fonts/arial.ttf", gSystem->Getenv("ROOTSYS"));
  mSize = 16;
  mDepthFac = 0.2;
}

/**************************************************************************/

void ZRlFont::SetFontFile(const Text_t* f)
{
  mFontFile = f;
  gSystem->ExpandPathName(mFontFile);
  StampReqTring(FID());
  EmitFontChangeRay();
}

void ZRlFont::EmitFontChangeRay()
{
  if (mQueen && mSaturn->AcceptsRays())
  {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_font_change, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

void ZRlFont::EmitSizeChangeRay()
{
  if (mQueen && mSaturn->AcceptsRays())
  {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_size_change, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

void ZRlFont::EmitDepthChangeRay()
{
  if (mQueen && mSaturn->AcceptsRays())
  {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_depth_change, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

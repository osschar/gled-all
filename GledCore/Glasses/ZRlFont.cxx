// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZRlFont
//
//

#include "ZRlFont.h"
#include "ZRlFont.c7"
#include <Glasses/ZQueen.h>

ClassImp(ZRlFont)

/**************************************************************************/

void ZRlFont::_init()
{
  mSize = 12;
}

/**************************************************************************/

void ZRlFont::SetFontFile(const Text_t* f)
{
  mFontFile = f;
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_font_change, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
  mStampReqTring = Stamp(FID());
}

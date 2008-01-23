// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PSMark
//
//

#include "PSMark.h"
#include "ParaSurf.h"
#include "PSMark.c7"

ClassImp(PSMark);

/**************************************************************************/

void PSMark::_init()
{
  mF = mG = mH = 0;
}

/**************************************************************************/

void PSMark::retrans()
{
  if (*mParaSurf)
  {
    mParaSurf->regularize_fg(&mF);
    mParaSurf->fgh2trans(&mF, mTrans);
  }
}

void PSMark::SetF(Float_t f)
{
  mF = f;
  retrans();
  mStampReqTrans = Stamp();
}

void PSMark::SetG(Float_t g)
{
  mG = g;
  retrans();
  mStampReqTrans = Stamp();

}

void PSMark::SetH(Float_t h)
{
  mH = h;
  retrans();
  mStampReqTrans = Stamp();
}

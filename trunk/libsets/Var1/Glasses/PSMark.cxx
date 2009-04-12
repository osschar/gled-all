// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PSMark.h"
#include "ParaSurf.h"
#include "PSMark.c7"

// PSMark

//__________________________________________________________________________
//
//

ClassImp(PSMark);

/**************************************************************************/

void PSMark::_init()
{
  mF = mG = mH = 0;
}

PSMark::PSMark(const Text_t* n, const Text_t* t) :
  ZNode(n,t)
{
  _init();
}

PSMark::PSMark(ParaSurf* ps, const Text_t* n, const Text_t* t) :
  ZNode(n,t), mParaSurf(ps)
{
  _init();
}

/**************************************************************************/

void PSMark::retrans(ParaSurf* ps)
{
  static const Exc_t _eh("PSMark::retrans ");

  ps->regularize_fg(&mF);
  ps->fgh2trans(&mF, mTrans);
}

void PSMark::SetF(Float_t f)
{
  static const Exc_t _eh("PSMark::SetF ");

  ParaSurf* ps = AssertParaSurf(_eh);
  if (f < ps->GetMinF()) f = ps->GetMinF();
  if (f > ps->GetMaxF()) f = ps->GetMaxF();
  mF = f;

  retrans(ps);

  mStampReqTrans = Stamp();
}

void PSMark::SetG(Float_t g)
{
  static const Exc_t _eh("PSMark::SetG ");

  ParaSurf* ps = AssertParaSurf(_eh);
  if (g < ps->GetMinG()) g = ps->GetMinG();
  if (g > ps->GetMaxG()) g = ps->GetMaxG();
  mG = g;

  retrans(ps);

  mStampReqTrans = Stamp();
}

void PSMark::SetH(Float_t h)
{
  static const Exc_t _eh("PSMark::SetH ");

  ParaSurf* ps = AssertParaSurf(_eh);
  mH = h;
  retrans(ps);

  mStampReqTrans = Stamp();
}

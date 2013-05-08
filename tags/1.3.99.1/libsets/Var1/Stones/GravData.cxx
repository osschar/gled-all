// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GravData.h"

// GravData

//______________________________________________________________________________
//
//

ClassImp(GravData);

void GravData::clear()
{
  fDir[0] = fDir[1] = fDir[2] = 0;
  fMag = fLDer = fTDer = fH = 0;
  fSafeTime = fSafeDistance = 0;
}

void GravData::add(Float_t f, GravData& gd)
{
  // fPos is not changed.
  // fH is added, but the caller should probably know better.
  // fDown is not added.

  Float_t fmag = f * gd.fMag;
  fDir[0] += fmag * gd.fDir[0];
  fDir[1] += fmag * gd.fDir[1];
  fDir[2] += fmag * gd.fDir[2];

  fLDer += f * gd.fLDer;
  fTDer += f * gd.fTDer;
  fH    += f * gd.fH;
}

//==============================================================================

void GravData::Combine(lGravFraction_t& fractions)
{
  clear();

  for (lGravFraction_i i = fractions.begin(); i != fractions.end(); ++i)
  {
    add(i->first, *i->second);
  }

  fMag = Dir().NormalizeAndReport();
}

//==============================================================================

void GravData::Print(Option_t *) const
{
  printf("pos=(%f %f %f), dir=(%f %f %f), mag=%f, l'=%f, t'=%f, h=%f\n",
         fPos[0], fPos[1], fPos[2], fDir[0], fDir[1], fDir[2],
         fMag, fLDer, fTDer, fH);
}


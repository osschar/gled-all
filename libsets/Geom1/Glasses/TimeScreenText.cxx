// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TimeScreenText
//
//

#include "TimeScreenText.h"
#include "TimeScreenText.c7"

ClassImp(TimeScreenText);

/**************************************************************************/

void TimeScreenText::_init()
{}

/**************************************************************************/

void TimeScreenText::TimeTick(Double_t t, Double_t dt)
{
  SetText(GForm(mFormat, t));
}

/**************************************************************************/

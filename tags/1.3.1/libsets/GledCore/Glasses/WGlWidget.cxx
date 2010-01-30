// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// WGlWidget
//
//

#include "WGlWidget.h"
#include "WGlWidget.c7"

ClassImp(WGlWidget);

/**************************************************************************/

void WGlWidget::_init()
{
  mDx = mDy = 0;
}

/**************************************************************************/

void WGlWidget::SetDaughterCbackAlpha(ZGlass* lens, Int_t recurse_lvl)
{
  // No locking for list, daughters done.

  Stepper<WGlWidget> stepper(this);
  while (stepper.step()) {
    stepper->SetCbackAlpha(lens);
    if (recurse_lvl != 0)
      stepper->SetDaughterCbackAlpha(lens, recurse_lvl - 1);
  }
}

void WGlWidget::SetDaughterCbackStuff(ZGlass* lens, Int_t recurse_lvl)
{
  // Set various members of daughter widgets depending on the contents
  // of their title:
  // 'LensName'  - set name
  // 'LensAlpha' - set callback alpha
  // 'LensBeta'  - set callback beta
  // Several tokens can be present, all will be processed.
  //
  // No locking for list, daughters done.

  Stepper<WGlWidget> stepper(this);
  while (stepper.step())
  {
    if (stepper->RefTitle().Contains("LensName"))
    {
      stepper->SetName(lens->GetName());
    }
    if (stepper->RefTitle().Contains("LensAlpha"))
    {
      stepper->SetCbackAlpha(lens);
    }
    if (stepper->RefTitle().Contains("LensBeta"))
    {
      stepper->SetCbackBeta(lens);
    }

    if (recurse_lvl != 0)
      stepper->SetDaughterCbackStuff(lens, recurse_lvl - 1);
  }
}

/**************************************************************************/

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletRnrMod.h"
#include "TabletRnrMod.c7"

// TabletRnrMod

//______________________________________________________________________________
//
//

ClassImp(TabletRnrMod);

//==============================================================================

void TabletRnrMod::_init()
{
  mMarkSize = 0.05;
  mPressCurveAlpha = 0;

  mInTouchColor.rgba(1, 0, 0, 1);
  mInProximityColor.rgba(0, 1, 0, 1);

  mPointColor.rgba(1, 0.5, 0, 1);
  mLineColor .rgba(0, 0.5, 1, 1);
  bRnrPoints = bRnrLine = true;
}

TabletRnrMod::TabletRnrMod(const Text_t* n, const Text_t* t) :
  ZRnrModBase(n, t)
{
  _init();
}

TabletRnrMod::~TabletRnrMod()
{}

//==============================================================================

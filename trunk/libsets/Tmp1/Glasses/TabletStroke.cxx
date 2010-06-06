// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStroke.h"
#include "TabletStroke.c7"

// TabletStroke

//______________________________________________________________________________
//
//

ClassImp(TabletStroke);

//==============================================================================

void TabletStroke::_init()
{}

TabletStroke::TabletStroke(const Text_t* n, const Text_t* t) :
  ZNode(n, t)
{
  _init();
}

TabletStroke::~TabletStroke()
{}

//==============================================================================

void TabletStroke::AddPoint(Float_t x, Float_t y)
{
  mPoints.push_back(HPointF(x, y, 0));
}

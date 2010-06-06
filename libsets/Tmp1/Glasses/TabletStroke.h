// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletStroke_H
#define Tmp1_TabletStroke_H

#include <Glasses/ZNode.h>
#include <Stones/HTrans.h>
#include <Stones/ZColor.h>

class TabletStroke : public ZNode
{
  MAC_RNR_FRIENDS(TabletStroke);

private:
  void _init();

protected:
  ZColor          mColor;    // X{PRGS} 7 ColorButt()
  vector<HPointF> mPoints;

public:
  TabletStroke(const Text_t* n="TabletStroke", const Text_t* t=0);
  virtual ~TabletStroke();

  void AddPoint(Float_t x, Float_t y);

#include "TabletStroke.h7"
  ClassDef(TabletStroke, 1);
}; // endclass TabletStroke

#endif

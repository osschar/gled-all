// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletStroke_H
#define Tmp1_TabletStroke_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/STabletPoint.h>

class TabletStroke : public ZNode
{
  MAC_RNR_FRIENDS(TabletStroke);

private:
  void _init();

protected:
  ZColor               mColor;    // X{PRGS} 7 ColorButt()
  vector<STabletPoint> mPoints;

public:
  TabletStroke(const Text_t* n="TabletStroke", const Text_t* t=0);
  virtual ~TabletStroke();

  void AddPoint(Float_t x, Float_t y, Float_t t, Float_t p);

  void Print(); //! X{E} 7 MButt()
  void MakeHisto(Int_t nbins=128, Float_t x_edge=0.05, Float_t y_edge=0.05); //! X{E} 7 MCWButt()

#include "TabletStroke.h7"
  ClassDef(TabletStroke, 1);
}; // endclass TabletStroke

#endif

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletRnrMod_H
#define Tmp1_TabletRnrMod_H

#include <Glasses/ZRnrModBase.h>
#include <Stones/ZColor.h>

class TabletRnrMod : public ZRnrModBase
{
  MAC_RNR_FRIENDS(TabletRnrMod);

private:
  void _init();

protected:
  // Common parameters
  Float_t       mMarkSize;         // X{GST}   7 Value(-range=>[0, 1,  1, 1000])
  Float_t       mPressCurveAlpha;  // X{GST}   7 Value(-range=>[0, 10, 1, 1000])

  // Reader parameters
  ZColor        mInTouchColor;     // X{PRGST} 7 ColorButt()
  ZColor        mInProximityColor; // X{PRGST} 7 ColorButt()

  // Stroke parameters
  ZColor        mPointColor;       // X{PRGST} 7 ColorButt(-join=>1)
  ZColor        mLineColor;        // X{PRGST} 7 ColorButt()
  Bool_t        bRnrPoints;        // X{GST}   7 Bool(-join=>1)
  Bool_t        bRnrSpheres;       // X{GST}   7 Bool()

public:
  TabletRnrMod(const Text_t* n="TabletRnrMod", const Text_t* t=0);
  virtual ~TabletRnrMod();

#include "TabletRnrMod.h7"
  ClassDef(TabletRnrMod, 1);
}; // endclass TabletRnrMod

#endif

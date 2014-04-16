// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletStroke_H
#define Tmp1_TabletStroke_H

#include <Glasses/ZNode.h>
#include <Stones/STabletPoint.h>

class TabletStroke : public ZNode
{
  MAC_RNR_FRIENDS(TabletStroke);
  friend class TabletStrokeList;

private:
  void _init();

protected:
  Float_t              mStartTime; // X{GS}   7 Value()
  vSTabletPoint_t      mPoints;
  vector<Int_t>        mBreaks;
  Bool_t               bInStroke;      //!
  Bool_t               bEnableDLAtEnd; //!

  Int_t get_num_points() const;
  void  get_draw_range(Int_t& min, Int_t& max) const;

  STabletPoint pre_sym_quadratic(Int_t i0, Int_t i1, Int_t i2) const;

public:
  TabletStroke(const Text_t* n="TabletStroke", const Text_t* t=0);
  virtual ~TabletStroke();

  // Interface for TabletReader
  void BeginStroke();
  void AddPoint(Float_t x, Float_t y, Float_t t, Float_t p);
  void EndStroke(Bool_t clip_trailing_zero_pressure_points);

  void MakeWSSeed(); // X{E} 7 MButt()

  Float_t MinT() const { return mPoints.empty() ? 0 : mPoints.front().t; }
  Float_t MaxT() const { return mPoints.empty() ? 0 : mPoints.back().t;  }

  void Print(); //! X{E} 7 MButt()
  void MakeHisto(Int_t nbins=128, Float_t x_edge=0.05, Float_t y_edge=0.05); //! X{ED} 7 MCWButt()
  void MakeDeltaHistos(Int_t nbins=128); //! X{ED} 7 MCWButt()

#include "TabletStroke.h7"
  ClassDef(TabletStroke, 1);
}; // endclass TabletStroke

#endif

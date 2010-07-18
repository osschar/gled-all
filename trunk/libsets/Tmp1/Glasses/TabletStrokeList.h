// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletStrokeList_H
#define Tmp1_TabletStrokeList_H

#include <Glasses/ZNode.h>
#include <Stones/GTSIsoMakerFunctor.h>

class STabletPoint;

#include <TKDTree.h>

class TabletStrokeList : public ZNode,
			 public GTSIsoMakerFunctor
{
  MAC_RNR_FRIENDS(TabletStrokeList);

public:
  enum PotentialAlgorithm_e
  {
    PA_StrongestPoint,
    PA_TwoStrongestPoints,
    PA_LinearInterpolation
  };

private:
  void _init();

protected:
  // Parameters for iso-triangulation
  PotentialAlgorithm_e mAlgorithm;     // X{GS} 7 PhonyEnum()
  Double_t             mWidth;         // X{GS} 7 Value(-range=>[  0,  1, 1, 1e6])
  Double_t             mPotentialExp;  // X{GS} 7 Value(-range=>[-10,  0, 1, 1e6])
  Double_t             mPressureAlpha; // X{GS} 7 Value(-range=>[  0, 10, 1, 1e6])
  Double_t             mMaxInterPoint; // X{GS} 7 Value(-range=>[  0, 1 , 1, 1e6])

  // Internal KD-tree and stuff for fast point search.
  TKDTreeIF             *mKDTree;    //!
  vector<STabletPoint*>  mPointRefs; //!
  vector<Float_t>        mArrX;      //!
  vector<Float_t>        mArrY;      //!
  Double_t               mSearchRad; //!

public:
  TabletStrokeList(const Text_t* n="TabletStrokeList", const Text_t* t=0);
  virtual ~TabletStrokeList();

  void MakeKDStuff();  // X{E} 7 MButt(-join=>1)
  void ClearKDStuff(); // X{E} 7 MButt()

  void PrintClose(Float_t x, Float_t y, Float_t rad);

  virtual void     GTSIsoBegin(Double_t iso_value);
  virtual Double_t GTSIsoFunc(Double_t x, Double_t y, Double_t z);
  virtual void     GTSIsoEnd();

#include "TabletStrokeList.h7"
  ClassDef(TabletStrokeList, 1);
}; // endclass TabletStrokeList

#endif

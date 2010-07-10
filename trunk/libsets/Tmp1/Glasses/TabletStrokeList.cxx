// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStrokeList.h"
#include "TabletStrokeList.c7"
#include "TabletStroke.h"
#include <Stones/STabletPoint.h>

// TabletStrokeList

//______________________________________________________________________________
//
//

ClassImp(TabletStrokeList);

//==============================================================================

void TabletStrokeList::_init()
{
  mExp   = -0.5;
  mWidth =  0.02;
}

TabletStrokeList::TabletStrokeList(const Text_t* n, const Text_t* t) :
  ZNode(n, t),
  mKDTree(0)
{
  _init();
}

TabletStrokeList::~TabletStrokeList()
{
  ClearKDStuff();
}

//==============================================================================

void TabletStrokeList::MakeKDStuff()
{
  if (mKDTree)
    ClearKDStuff();

  Stepper<TabletStroke> stepper(this);
  while (stepper.step())
  {
    Int_t min, max;
    stepper->get_draw_range(min, max);
    Int_t n_new = mPointRefs.size() + max - min + 1;
    mPointRefs.reserve(n_new);
    mArrX.reserve(n_new);
    mArrY.reserve(n_new);
    for (Int_t i = min; i <= max; ++i)
    {
      STabletPoint &p = stepper->mPoints[i];
      mPointRefs.push_back(&p);
      mArrX.push_back(p.x);
      mArrY.push_back(p.y);
    }
  }

  mKDTree = new TKDTreeIF(mArrX.size(), 2, 1);
  mKDTree->SetData(0, &mArrX[0]);
  mKDTree->SetData(1, &mArrY[0]);
  mKDTree->Build();  
}

void TabletStrokeList::ClearKDStuff()
{
  if (mKDTree)
  {
    delete mKDTree; mKDTree = 0;
    mPointRefs.clear();
    mArrX.clear();
    mArrY.clear();
  }
}

//==============================================================================

void TabletStrokeList::PrintClose(Float_t x, Float_t y, Float_t rad)
{
  if (mKDTree == 0)
    return;

  Float_t       point[2] = { x, y };
  vector<Int_t> res;

  mKDTree->FindInRange(point, rad, res);

  printf("TabletStrokeList::PrintClose npoints=%d, all stored=%d\n",
	 (Int_t) res.size(), (Int_t) mPointRefs.size());
}

//==============================================================================

void TabletStrokeList::GTSIsoBegin(Double_t iso_value)
{
  if (mKDTree == 0)
    MakeKDStuff();

  mIsoValue = iso_value;
}

namespace
{
  Double_t sqr(Double_t x) { return x*x; }
}

Double_t TabletStrokeList::GTSIsoFunc(Double_t x, Double_t y, Double_t z)
{
  Float_t       in_point[2] = { x, y };
  vector<Int_t> result;

  mKDTree->FindInRange(in_point, 5*mWidth, result);

  Double_t vm[2] = { 0, 0 };

  for (vector<Int_t>::iterator pi = result.begin(); pi != result.end(); ++pi)
  {
    STabletPoint &p = * mPointRefs[*pi];
    Double_t v = TMath::Power((sqr(x - p.x) + sqr(y - p.y) + sqr(z)) / sqr(mWidth*p.p), mExp);
    if (v > vm[0])
    {
      vm[1] = vm[0];
      vm[0] = v;
    }
    else if (v > vm[1])
    {
      vm[1] = v;
    }
  }

  return vm[0] + vm[1];
}

void TabletStrokeList::GTSIsoEnd()
{}

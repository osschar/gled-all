// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStroke.h"
#include "TabletStroke.c7"

#include "TH2I.h"
#include "TSystem.h"
#include "TROOT.h"

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

void TabletStroke::MakeHisto(Int_t nbins)
{
  Float_t min[2], max[2];

  min[0] = max[0] = mPoints[0].x;
  min[1] = max[1] = mPoints[0].y;

  for (Int_t i = 1; i < mPoints.size(); ++i)
  {
    for (Int_t c = 0; c < 2; ++c)
    {
      min[c] = TMath::Min(min[c], mPoints[i][c]);
      max[c] = TMath::Max(max[c], mPoints[i][c]);
    }
  }

  TH2I *h = new TH2I("Points", "Points", nbins, min[0], max[0], nbins, min[1], max[1]);
  for (Int_t i = 0; i < mPoints.size(); ++i)
  {
    h->Fill(mPoints[i].x, mPoints[i].y);
  }

  gROOT->ProcessLine(GForm("((TH2I*)0x%lx)->Draw()", h));
  //h->Draw();
}

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStroke.h"
#include "TabletStroke.c7"

#include "GledView/GledGUI.h"

// #include "TSpline.h"
#include "TCanvas.h"
#include "TH2I.h"

// TabletStroke

//______________________________________________________________________________
//
//

ClassImp(TabletStroke);

//==============================================================================

void TabletStroke::_init()
{
  mStartTime = 0;
  bInStroke = false;
  mLineColor.rgba(0, 0.5, 1, 1);
  // mSplineX = mSplineY = 0;
}

TabletStroke::TabletStroke(const Text_t* n, const Text_t* t) :
  ZNode(n, t)
{
  _init();
}

TabletStroke::~TabletStroke()
{}

//==============================================================================

void TabletStroke::get_draw_range(Int_t& min, Int_t& max)
{
  if (mPoints.empty())
  {
    min = max = 0;
  }
  else
  {
    min = 1;
    max = mPoints.size() - (bInStroke ? 1 : 2);
  }
}

void TabletStroke::BeginStroke()
{
  static const Exc_t _eh("TabletStroke::BeginStroke ");

  if (bInStroke)
    throw _eh + "Already in stroke.";

  bInStroke = true;
  mPoints.clear();
  mPoints.push_back(STabletPoint());
}

void TabletStroke::AddPoint(Float_t x, Float_t y, Float_t t, Float_t p)
{
  static const Exc_t _eh("TabletStroke::AddPoint ");

  if (!bInStroke)
    throw _eh + "Not in stroke.";

  mPoints.push_back(STabletPoint(x, y, 0, t, p));
}

void TabletStroke::EndStroke(Bool_t clip_trailing_zero_pressure_points)
{
  static const Exc_t _eh("TabletStroke::EndStroke ");

  if (!bInStroke)
    throw _eh + "Not in stroke.";

  if (clip_trailing_zero_pressure_points)
  {
    vSTabletPoint_i i = mPoints.end();
    do
    {
      --i;
    }
    while (i != mPoints.begin() && i->p == 0);
    ++i;
    mPoints.erase(i, mPoints.end());
  }
  mPoints.push_back(STabletPoint());
  bInStroke = false;
}

//==============================================================================

void TabletStroke::MakeSplines()
{
  // if (mSplineX) delete mSplineX;
  // if (mSplineY) delete mSplineY;

  // Int_t np = mPoints.size();

  // vector<Double_t> vx(np), vy(np), vt(np);

  // for (Int_t i = 0; i < np; ++i)
  // {
  //   const STabletPoint &p = mPoints[i];
  //   vx[i] = p.x;
  //   vy[i] = p.y;
  //   vt[i] = p.t;
  // }

  // mSplineX = new TSpline3("x", &vt[0], &vx[0], np);
  // mSplineY = new TSpline3("y", &vt[0], &vy[0], np);
}

//==============================================================================

void TabletStroke::Print()
{
  Int_t np = mPoints.size();
  printf("Foo NP=%d\n", np);
  for (Int_t i = 0; i < np; ++i)
  {
    STabletPoint &p = mPoints[i];
    printf("  %3d  %8.4f %8.4f %8.4f, %8.4f; %6.4f\n",
	   i, p.x, p.y, p.z, p.t, p.p);
  }
}

//==============================================================================

void TabletStroke::MakeHisto(Int_t nbins, Float_t x_edge, Float_t y_edge)
{
  Float_t min[2], max[2];

  min[0] = max[0] = mPoints[0].x;
  min[1] = max[1] = mPoints[0].y;

  Int_t np = mPoints.size();
  for (Int_t i = 1; i < np; ++i)
  {
    for (Int_t c = 0; c < 2; ++c)
    {
      min[c] = TMath::Min(min[c], mPoints[i][c]);
      max[c] = TMath::Max(max[c], mPoints[i][c]);
    }
  }
  Float_t xe = x_edge * (max[0] - min[0]);
  Float_t ye = y_edge * (max[1] - min[1]);

  TH2I *h = new TH2I("Points", "Points",
		     nbins, min[0] - xe, max[0] + xe,
		     nbins, min[1] - ye, max[1] + ye);
  for (Int_t i = 0; i < np; ++i)
  {
    h->Fill(mPoints[i].x, mPoints[i].y);
  }

  if (gPad == 0)
  {
    printf("Requesting canvas ... expect trouble ;)\n");
    TCanvas *c = GledGUI::theOne->NewCanvas("foo", "bar");
    printf("Got back %p\n", c);
  }
  h->Draw();
}

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStroke.h"
#include "TabletStroke.c7"

#include "Gled/XTReqCanvas.h"

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
  // Override settings from ZGlass
  bUseDispList = true;

  mStartTime = 0;
  bInStroke = false;
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
    min = max = -1;
  }
  else
  {
    min = 1;
    max = mPoints.size() - (bInStroke ? 1 : 2);
  }
}

//------------------------------------------------------------------------------

STabletPoint TabletStroke::pre_sym_quadratic(Int_t i0, Int_t i1, Int_t i2) const
{
  STabletPoint d1 = mPoints[i1] - mPoints[i0];
  STabletPoint d2 = mPoints[i2] - mPoints[i0];

  return 2.0f * d1.t / (d2.t - d1.t) * (d1.t * d2 - d2.t * d1) - d1;
}

//==============================================================================

void TabletStroke::BeginStroke()
{
  static const Exc_t _eh("TabletStroke::BeginStroke ");

  if (bInStroke)
    throw _eh + "Already in stroke.";

  bInStroke = true;
  if (bUseDispList)
  {
    bEnableDLAtEnd = true;
    bUseDispList   = false;
  }

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

  Int_t NP = mPoints.size() - 2;
  if (NP > 2)
  {
    mPoints.front() = mPoints[1]  + pre_sym_quadratic(1, 2, 3);
    mPoints.back()  = mPoints[NP] + pre_sym_quadratic(NP, NP - 1, NP - 2);
  }
  else if (NP > 1)
  {
    mPoints.front() = 2.0f * mPoints[1] - mPoints[2];
    mPoints.back()  = 2.0f * mPoints[2] - mPoints[1];
  }
  else
  {
    mPoints.front() = mPoints.back() = mPoints[1];
  }

  bInStroke = false;
  if (bEnableDLAtEnd)
  {
    bUseDispList = true;
  }
}

//==============================================================================

#include <Glasses/WSSeed.h>
#include <Glasses/WSPoint.h>

void TabletStroke::MakeWSSeed()
{
  WSSeed *seed = new WSSeed();
  seed->SetFat(true);
  mQueen->CheckIn(seed);
  Add(seed);

  Int_t NP = mPoints.size() - 2;
  for (Int_t i = 1; i <= NP; ++i)
  {
    STabletPoint &prev = mPoints[i-1];
    STabletPoint &curr = mPoints[i];
    STabletPoint &next = mPoints[i+1];
    STabletPoint delta = next - prev;
    // Float_t      d_mag = delta.Mag();

    WSPoint *p = new WSPoint(GForm("Point %d", i));
    p->SetPos(curr.x, curr.y, curr.z);
    p->RotateLF(1, 2, delta.Phi());
    p->SetW(0.02f * curr.p);
    // p->SetS(0.01f * delta.p / d_mag);
    // p->SetT(10.0f * d_mag / delta.t);

    mQueen->CheckIn(p);
    seed->Add(p);
  }
}

//==============================================================================

void TabletStroke::Print()
{
  Int_t np = mPoints.size();
  printf("Foo NP=%d\n", np);
  for (Int_t i = 0; i < np; ++i)
  {
    STabletPoint &p = mPoints[i];
    printf("  %3d  %9.6f %9.6f %9.6f, %9.6f; %6.4f\n",
	   i, p.x, p.y, p.z, p.t, p.p);
  }
}

//==============================================================================

void TabletStroke::MakeHisto(Int_t nbins, Float_t x_edge, Float_t y_edge)
{
  Int_t np = mPoints.size() - 1;
  if (np < 1)
    return;

  Float_t min[2], max[2];

  min[0] = max[0] = mPoints[1].x;
  min[1] = max[1] = mPoints[1].y;

  for (Int_t i = 2; i < np; ++i)
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
  for (Int_t i = 1; i < np; ++i)
  {
    h->Fill(mPoints[i].x, mPoints[i].y);
  }

  TCanvas *canvas = XTReqCanvas::Request("Stroke", "Stroke");
  h->Draw();
  XTReqPadUpdate::Update(canvas);
}

void TabletStroke::MakeDeltaHistos(Int_t nbins)
{
  Float_t max_d = 0, max_v = 0, max_t = 0;

  Int_t np = mPoints.size() - 1;
  for (Int_t i = 1; i < np; ++i)
  {
    STabletPoint d = mPoints[i+1] - mPoints[i];
    Float_t dist = d.Mag();
    Float_t vel  = dist / d.t;
    max_d = TMath::Max(max_d, dist);
    max_v = TMath::Max(max_v, vel);
    max_t = TMath::Max(max_t, d.t);
  }
  max_d *= 1.001f;
  max_v *= 1.001f;
  max_t *= 1.001f;
 
  TH1I *hd = new TH1I("Distance", "Distance", nbins, 0, max_d);
  TH1I *hv = new TH1I("Velocity", "Velocity", nbins, 0, max_v);
  TH1I *ht = new TH1I("Delta T",  "Delta T",  nbins, 0, max_t);
  // d .vs. v correlation
  // TH2I *hc = new TH2I(""

  for (Int_t i = 1; i < np; ++i)
  {
    STabletPoint d = mPoints[i+1] - mPoints[i];
    Float_t dist = d.Mag();
    Float_t vel  = dist / d.t;
    hd->Fill(dist);
    hv->Fill(vel);
    ht->Fill(d.t);
  }

  TCanvas *canvas = XTReqCanvas::Request("StrokeDeltas", "Stroke Deltas", 1024, 768, 2, 2);
  canvas->cd(1); hd->Draw();
  canvas->cd(2); hv->Draw();
  canvas->cd(3); ht->Draw();
  XTReqPadUpdate::Update(canvas);
}

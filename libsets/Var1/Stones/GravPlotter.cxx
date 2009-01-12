// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GravPlotter.h"

#include <Glasses/ParaSurf.h>

#include <Stones/TringTvor.h>
#include <Glasses/TriMesh.h>

#include <Glasses/Tringula.h>

#include <Math/ParamFunctor.h>
#include <TF2.h>
#include <TH1.h>
#include <TCanvas.h>

// GravPlotter

//______________________________________________________________________________
//
//

ClassImp(GravPlotter);

//==============================================================================

void GravPlotter::_init()
{
  fNDiv = 100;
  fMag = fH = fAngle = fXDir = fZDir = fGrad = 0;
  fDrawOpt = "glsurf2";
}


GravPlotter::GravPlotter(const Text_t* n, const Text_t* t) :
  TNamed(n, t)
{
  _init();
}

GravPlotter::GravPlotter(ParaSurf* ps, Float_t fac, Int_t n_div) :
  TNamed("GravPlotter", "Foo"),
  fNDiv (n_div)
{
  _init();

  fPS = ps;

  Float_t l = fac * ps->CharacteristicLength();

  fXm = fZm = -l;
  fXM = fZM =  l;

  fMag   = make_tf2("grav_mag",   "Magnitude", &GravPlotter::grav_mag);
  fH     = make_tf2("grav_h",     "h",         &GravPlotter::grav_h);
  fAngle = make_tf2("grav_angle", "Down-Grav angle", &GravPlotter::grav_angle);
  fXDir  = make_tf2("grav_xdir",  "x-dir",     &GravPlotter::grav_xdir);
  fZDir  = make_tf2("grav_zdir",  "z-dir",     &GravPlotter::grav_zdir);
  fGrad  = make_tf2("grav_grad",   "gradient", &GravPlotter::grav_grad);
}

GravPlotter::~GravPlotter()
{}

//==============================================================================

TF2* GravPlotter::make_tf2(const Text_t* n, const Text_t* t, tf2_foo foo)
{
  TF2* f = new TF2(n, ROOT::Math::ParamFunctor(this, foo), fXm, fXM, fZm, fZM, 0);
  f->SetTitle(t);
  f->SetNpx(fNDiv);
  f->SetNpy(fNDiv);
  return f;
}

void GravPlotter::draw_tf2(TF2* f)
{
  f->Draw(fDrawOpt);

  gPad->Update();

  TH1* h = f->GetHistogram();
  h->GetXaxis()->SetTitle("X");
  h->GetYaxis()->SetTitle("Z");
  h->GetZaxis()->SetTitle(f->GetTitle());
  h->GetXaxis()->SetTitleOffset(1.5);
  h->GetYaxis()->SetTitleOffset(1.5);
  h->GetZaxis()->SetTitleOffset(1.5);

  gPad->Modified();
}

//==============================================================================

void GravPlotter::fill_gd(Double_t *x)
{
  Float_t pos[3] = { x[0], 0, x[1] };
  fPS->pos2grav(pos, fGD);
}

Double_t GravPlotter::grav_mag(Double_t *x, Double_t *params)
{
  fill_gd(x);
  return fGD.fMag;
}

Double_t GravPlotter::grav_h(Double_t *x, Double_t *params)
{
  fill_gd(x);
  return fGD.fH;
}

Double_t GravPlotter::grav_angle(Double_t *x, Double_t *params)
{
  fill_gd(x);
  return TMath::RadToDeg() * TMath::ACos(fGD.Dir() | fGD.Down());
}

Double_t GravPlotter::grav_xdir(Double_t *x, Double_t *params)
{
  fill_gd(x);
  return fGD.fDir[0];
}

Double_t GravPlotter::grav_zdir(Double_t *x, Double_t *params)
{
  fill_gd(x);
  return fGD.fDir[2];
}

Double_t GravPlotter::grav_grad(Double_t *x, Double_t *params)
{
  fill_gd(x);
  return sqrtf(fGD.fLDer*fGD.fLDer + fGD.fTDer*fGD.fTDer);
}

//==============================================================================

TCanvas* GravPlotter::StandardCanvas()
{
  // This would best be called from ROOT prompt.

  TCanvas* c = new TCanvas(GForm("gl%s", fPS->GetName()),
                           GForm("Grav plots for %s", fPS->GetName()));
  c->Divide(3, 2);

  Int_t i = 1;
  c->cd(i++); draw_tf2(fMag);
  c->cd(i++); draw_tf2(fH);
  c->cd(i++); draw_tf2(fAngle);
  c->cd(i++); draw_tf2(fXDir);
  c->cd(i++); draw_tf2(fZDir);
  c->cd(i++); draw_tf2(fGrad);

  c->Modified();
  c->Update();

  return c;
}

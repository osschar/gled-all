// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_GravPlotter_H
#define Var1_GravPlotter_H

#include <TNamed.h>

#include "GravData.h"

class ParaSurf;
class TriMesh;

class TF2;
class TCanvas;

class GravPlotter : public TNamed
{
private:
  void _init();

protected:
  ParaSurf* fPS;
  GravData  fGD;

  Float_t   fXm, fXM, fZm, fZM;
  Int_t     fNDiv;

  void fill_gd(Double_t *x);

  typedef Double_t (GravPlotter::*tf2_foo)(Double_t*, Double_t*);

  TF2* make_tf2(const Text_t* n, const Text_t* t, tf2_foo foo);
  void draw_tf2(TF2* f);

public:
  TF2* fMag;
  TF2* fH;
  TF2* fAngle;
  TF2* fXDir;
  TF2* fZDir;
  TF2* fGrad;

  TString fDrawOpt;

  Double_t grav_mag  (Double_t *x, Double_t *params);
  Double_t grav_h    (Double_t *x, Double_t *params);
  Double_t grav_angle(Double_t *x, Double_t *params);
  Double_t grav_xdir (Double_t *x, Double_t *params);
  Double_t grav_zdir (Double_t *x, Double_t *params);
  Double_t grav_grad (Double_t *x, Double_t *params);

public:
  GravPlotter(const Text_t* n="GravPlotter", const Text_t* t=0);
  GravPlotter(ParaSurf* ps, Float_t fac=2, Int_t n_div=120);

  virtual ~GravPlotter();

  TCanvas* StandardCanvas();

#include "GravPlotter.h7"
  ClassDef(GravPlotter, 1);
}; // endclass GravPlotter

#endif

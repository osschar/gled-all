// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GTSIsoMaker
//
//

#include "GTSIsoMaker.h"
#include "GTSIsoMaker.c7"

#include <GTS/GTS.h>

#include <TF3.h>

ClassImp(GTSIsoMaker)

/**************************************************************************/

void GTSIsoMaker::_init()
{
  // !!!! Set all links to 0 !!!!
  mTarget = 0;
  mAlgo   = A_Cartesian;
  mValue  = 0;
  mXmin = mYmin = mZmin = -1;
  mXmax = mYmax = mZmax =  1;
  mXdiv = mYdiv = mZdiv = 20;
}

/**************************************************************************/

namespace {
  void form_to_plane(GTS::gdouble **a, GTS::GtsCartesianGrid g,
		     GTS::guint i, GTS::gpointer data)
  {
    Double_t z = g.z;
    // printf("form_to_plane called w/ i=%d => z=%lf\n", i, z);
    TF3& formula = *((TF3*)data);

    Double_t x = g.x;
    for(int nx=0; nx<g.nx; ++nx) {
      Double_t y = g.y;
      for(int ny=0; ny<g.ny; ++ny) {
	a[nx][ny] = formula.Eval(x, y, z);
	y += g.dy;
      }
      x += g.dx;
    }
  }
};

void GTSIsoMaker::MakeSurface()
{  
  static const string _eh("GTSIsoMaker::MakeSurface ");

  using namespace GTS;

  GTSurf* target = mTarget;
  if(target == 0) {
    throw(_eh + "Link Target should be set.");
  }

  TF3 formula(GForm("GTSIsoMaker_%d", GetSaturnID()), mFormula.Data(),
	      mXmin, mXmax, mYmin, mYmax, mZmin, mZmax);

  GtsCartesianGrid grid = {
    mXdiv + 1, mYdiv + 1, mZdiv + 1,
    mXmin, (mXmax - mXmin) / mXdiv,
    mYmin, (mYmax - mYmin) / mYdiv,
    mZmin, (mZmax - mZmin) / mZdiv
  };

  GtsSurface* s = MakeDefaultSurface();

  switch(mAlgo) {
  case A_Cartesian: 
    gts_isosurface_cartesian(s, grid, form_to_plane, &formula, mValue);
    break;
  case A_Tetra:
    gts_isosurface_tetra(s, grid, form_to_plane, &formula, mValue);
    break;
  case A_TetraBounded:
    gts_isosurface_tetra_bounded(s, grid, form_to_plane, &formula, mValue);
    break;
  case A_TetraBCL:
    gts_isosurface_tetra_bcl(s, grid, form_to_plane, &formula, mValue);
    break;
  }

  target->WriteLock();
  target->ReplaceSurface(s);
  target->WriteUnlock();
}

/**************************************************************************/

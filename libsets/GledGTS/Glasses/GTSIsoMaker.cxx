// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GTSIsoMaker
//
//

#include "GTSIsoMaker.h"
#include "GTSIsoMaker.c7"
#include <Stones/GTSIsoMakerFunctor.h>
#include <GTS/GTS.h>

#include <TF3.h>

ClassImp(GTSIsoMaker);

/**************************************************************************/

void GTSIsoMaker::_init()
{
  mTarget = 0;
  mAlgo   = A_Cartesian;
  mValue  = 0;
  mXmin = mYmin = mZmin = -1;
  mXmax = mYmax = mZmax =  1;
  mXdiv = mYdiv = mZdiv = 20;
  bInvertCartesian = bInvertTetra = false;
}

/**************************************************************************/

void GTSIsoMaker::SetXAxis(Float_t min, Float_t max, UShort_t div)
{
  mXmin = min; mXmax = max; mXdiv = div;
  Stamp(FID());
}

void GTSIsoMaker::SetYAxis(Float_t min, Float_t max, UShort_t div)
{
  mYmin = min; mYmax = max; mYdiv = div;
  Stamp(FID());
}

void GTSIsoMaker::SetZAxis(Float_t min, Float_t max, UShort_t div)
{
  mZmin = min; mZmax = max; mZdiv = div;
  Stamp(FID());
}

/**************************************************************************/

namespace
{
  void form_to_plane(GTS::gdouble **a, GTS::GtsCartesianGrid g,
		     GTS::guint i, GTS::gpointer data)
  {
    Double_t z = g.z;
    // printf("form_to_plane called w/ i=%d => z=%lf\n", i, z);
    TF3& formula = *((TF3*)data);

    Double_t x = g.x;
    for(unsigned int nx=0; nx<g.nx; ++nx) {
      Double_t y = g.y;
      for(unsigned int ny=0; ny<g.ny; ++ny) {
	a[nx][ny] = formula.Eval(x, y, z);
	y += g.dy;
      }
      x += g.dx;
    }
  }

  void functor_to_plane(GTS::gdouble **a, GTS::GtsCartesianGrid g,
			GTS::guint i, GTS::gpointer data)
  {
    Double_t z = g.z;
    // printf("form_to_plane called w/ i=%d => z=%lf\n", i, z);
    GTSIsoMakerFunctor &functor = *((GTSIsoMakerFunctor*)data);

    Double_t x = g.x;
    for(unsigned int nx=0; nx<g.nx; ++nx) {
      Double_t y = g.y;
      for(unsigned int ny=0; ny<g.ny; ++ny) {
	a[nx][ny] = functor.GTSIsoFunc(x, y, z);
	y += g.dy;
      }
      x += g.dx;
    }
  }
};

void GTSIsoMaker::MakeSurface()
{
  static const Exc_t _eh("GTSIsoMaker::MakeSurface ");

  using namespace GTS;

  GTSurf* target = *mTarget;
  if (target == 0)
  {
    throw _eh + "Link Target should be set.";
  }

  GTS::gpointer            user_data = 0;
  GTS::GtsIsoCartesianFunc user_func = 0;

  GTSIsoMakerFunctor *functor = dynamic_cast<GTSIsoMakerFunctor*>(*mFunctor);
  TF3                *formula = 0;

  if (mFunctor != 0 && functor == 0)
  {
    throw _eh + "Link Functor is set, but it is not a GTSIsoMakerFunctor.";
  }

  if (functor)
  {
    user_data = functor;
    user_func = functor_to_plane;
  }
  else
  {
    formula = new TF3;
    formula->Compile(mFormula);
    formula->SetRange(mXmin, mXmax, mYmin, mYmax, mZmin, mZmax);
    user_data = formula;
    user_func = form_to_plane;
  }

  GtsCartesianGrid grid = {
    mXdiv + 1, mYdiv + 1, mZdiv + 1,
    mXmin, (mXmax - mXmin) / mXdiv,
    mYmin, (mYmax - mYmin) / mYdiv,
    mZmin, (mZmax - mZmin) / mZdiv
  };

  GtsSurface* s = MakeDefaultSurface();

  if (functor)
  {
    functor->GTSIsoBegin(mValue);
  }

  switch (mAlgo)
  {
  case A_Cartesian:
    gts_isosurface_cartesian(s, grid, user_func, user_data, mValue);
    break;
  case A_Tetra:
    gts_isosurface_tetra(s, grid, user_func, user_data, mValue);
    break;
  case A_TetraBounded:
    gts_isosurface_tetra_bounded(s, grid, user_func, user_data, mValue);
    break;
  case A_TetraBCL:
    gts_isosurface_tetra_bcl(s, grid, user_func, user_data, mValue);
    break;
  }

  if (functor)
  {
    functor->GTSIsoEnd();
  }
  else
  {
    delete formula;
  }

  if ((mAlgo == A_Cartesian && bInvertCartesian) ||
      (mAlgo >  A_Cartesian && bInvertTetra))
  {
    GTS::InvertSurface(s);
  }

  target->WriteLock();
  target->ReplaceSurface(s);
  target->WriteUnlock();
}

/**************************************************************************/

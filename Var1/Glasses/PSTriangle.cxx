// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PSTriangle
//
//

#include "PSTriangle.h"
#include "PSTriangle.c7"

#include <Opcode/Opcode.h>

#include <TRandom.h>

ClassImp(PSTriangle)

/**************************************************************************/

void PSTriangle::_init()
{
  mX0 = -0.5f; mY0 = -SQRT3/6.0f;
  mX1 =  0.5f; mY1 =  mY0;
  mX2 =  0.0f; mY2 =  SQRT3/3.0f;

  mEdgePlanes = new Opcode::Plane[3];
}

PSTriangle::PSTriangle(const Text_t* n, const Text_t* t) :
  ParaSurf(n,t)
{
  _init();
}

PSTriangle::~PSTriangle()
{
  delete [] mEdgePlanes;
}

/**************************************************************************/

void PSTriangle::Scale(Float_t s)
{
  mX0 *= s; mY0 *= s;
  mX1 *= s; mY1 *= s;
  mX2 *= s; mY2 *= s;
}

/**************************************************************************/

void PSTriangle::SetupEdgePlanes()
{
  // !!! should be called from AdEnlightenment

  using namespace Opcode;
  Point v0(mX0, mY0, 0), v1(mX1, mY1, 0), v2(mX2, mY2, 0);
  Point up(0, 0, 1);
  mEdgePlanes[0].Set(v0, v1, up + v0 + (v1 - v0));
  mEdgePlanes[1].Set(v1, v2, up + v1 + (v2 - v1));
  mEdgePlanes[2].Set(v2, v0, up + v2 + (v0 - v2));
}

/**************************************************************************/

void PSTriangle::originpos(Float_t* x)
{
  using namespace Opcode;
  Point opos(mX0, mY0, 0);
  Point udir(mX1, mY1, 0); udir.Sub(opos);
  Point vdir(mX2, mY2, 0); vdir.Sub(opos);
  ((Point*)x)->Mac2(opos, udir, 0.25f, vdir, 0.25f);
}

/**************************************************************************/

void PSTriangle::pos2fgh(const Float_t* x, Float_t* f)
{
  f[0] = x[0];
  f[1] = x[1];
  f[2] = x[2];
}

void PSTriangle::fgh2pos(const Float_t* f, Float_t* x)
{
  x[0] = f[0];
  x[1] = f[1];
  x[2] = f[2];
}

void PSTriangle::fgh2fdir(const Float_t* f, Float_t* d)
{
  d[0] = 1;
  d[1] = 0;
  d[2] = 0;
}

void PSTriangle::fgh2gdir(const Float_t* f, Float_t* d)
{
  d[0] = 0;
  d[1] = 1;
  d[2] = 0;
}

void PSTriangle::fgh2hdir(const Float_t* f, Float_t* d)
{
  d[0] = 0;
  d[1] = 0;
  d[2] = 1;
}

/**************************************************************************/

void PSTriangle::pos2hdir(const Float_t* x, Float_t* d)
{
  // Return 'up' direction.

  d[0] = 0;
  d[1] = 0;
  d[2] = 1;
}
 
Float_t PSTriangle::pos2hray(const Float_t* x, Opcode::Ray& r)
{
  // Setup ray r for given postition x so that the ray origin is above
  // the surface and its direction/lenght ascertain the surface will
  // be intersected.
  // Returns distance the ray-origin was shifted from initial pos.

  pos2hdir(x, r.mDir);
  Float_t dist = mMaxH - x[2] + mEpsilon;
  r.mOrig.Set(x);
  r.mOrig.TMac(r.mDir, dist);
  r.mDir.Neg();
  return dist;
}

/**************************************************************************/

void PSTriangle::random_fgh(TRandom& rnd, Float_t* f)
{
  using namespace Opcode;
  Point opos(mX0, mY0, 0);
  Point udir(mX1, mY1, 0); udir.Sub(opos);
  Point vdir(mX2, mY2, 0); vdir.Sub(opos);
  Float_t u = rnd.Uniform(0, 1);
  Float_t v = rnd.Uniform(0, 1 - u);
  ((Point*)f)->Mac2(opos, udir, u, vdir, v);
}

void PSTriangle::random_pos(TRandom& rnd, Float_t* x)
{
  using namespace Opcode;
  Point opos(mX0, mY0, 0);
  Point udir(mX1, mY1, 0); udir.Sub(opos);
  Point vdir(mX2, mY2, 0); vdir.Sub(opos);
  Float_t u = rnd.Uniform(0, 1);
  Float_t v = rnd.Uniform(0, 1 - u);
  ((Point*)x)->Mac2(opos, udir, u, vdir, v);
}

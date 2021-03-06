// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PSTriangle
//
//

#include "PSTriangle.h"
#include "PSTriangle.c7"
#include <Stones/GravData.h>

#include <Opcode/Opcode.h>

#include <TRandom.h>

ClassImp(PSTriangle);

/**************************************************************************/

void PSTriangle::_init()
{
  mX0 = -0.5f; mY0 = -Opcode::SQRT3/6.0f;
  mX1 =  0.5f; mY1 =  mY0;
  mX2 =  0.0f; mY2 =  Opcode::SQRT3/3.0f;

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

Float_t PSTriangle::Surface()
{
  using namespace Opcode;
  Point opos(mX0, mY0, 0);
  Point udir(mX1, mY1, 0); udir.Sub(opos);
  Point vdir(mX2, mY2, 0); vdir.Sub(opos);
  return 0.5f * opos.Cross(udir, vdir).Magnitude();
}

/******************************************************************************/

void PSTriangle::origin_fgh(Float_t* f)
{
  f[0] = Opcode::INV3*(mX0 + mX1 + mX2);
  f[1] = Opcode::INV3*(mY0 + mY1 + mY2);
  f[2] = 0;
}

void PSTriangle::origin_pos(Float_t* x)
{
  x[0] = Opcode::INV3*(mX0 + mX1 + mX2);
  x[1] = Opcode::INV3*(mY0 + mY1 + mY2);
  x[2] = 0;
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

void PSTriangle::pos2grav(const Float_t* x, GravData& gd)
{
  // Here we could have three vectors at the vertices of the triangle
  // and interpolate.
  // Then would need also intepolated height, or what?

  gd.fPos[0] = x[0]; gd.fPos[1] = x[1]; gd.fPos[2] = x[2];
  gd.fDir[0] = 0;    gd.fDir[1] = 0;    gd.fDir[2] = -1;

  gd.fMag  = mGravAtSurface;
  gd.fLDer = 0;
  gd.fTDer = 0;
  gd.fH    = x[2];
  gd.fDown[0] = gd.fDown[1] = 0; gd.fDown[2] = -1;
}

/**************************************************************************/

void PSTriangle::random_fgh(TRandom& rnd, Float_t* f)
{
  using namespace Opcode;
  Point opos(mX0, mY0, 0);
  Point udir(mX1, mY1, 0); udir.Sub(opos);
  Point vdir(mX2, mY2, 0); vdir.Sub(opos);
  Float_t u = rnd.Uniform(0, 1);
  Float_t v = rnd.Uniform(0, 1);
  if (u + v > 1) { u = 1.0f - u; v = 1.0f - v; }
  ((Point*)f)->Mac2(opos, udir, u, vdir, v);
}

void PSTriangle::random_pos(TRandom& rnd, Float_t* x)
{
  using namespace Opcode;
  Point opos(mX0, mY0, 0);
  Point udir(mX1, mY1, 0); udir.Sub(opos);
  Point vdir(mX2, mY2, 0); vdir.Sub(opos);
  Float_t u = rnd.Uniform(0, 1);
  Float_t v = rnd.Uniform(0, 1);
  if (u + v > 1) { u = 1.0f - u; v = 1.0f - v; }
  ((Point*)x)->Mac2(opos, udir, u, vdir, v);
}

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PSRectangle
//
//

#include "PSRectangle.h"
#include "PSRectangle.c7"

#include "TriMesh.h"

#include <Opcode/Opcode.h>

#include <TRandom.h>

ClassImp(PSRectangle)

/**************************************************************************/

void PSRectangle::_init()
{
  mMinX = 0; mMaxX = 1;
  mMinY = 0; mMaxY = 1;

  mEdgePlanes = new Opcode::Plane[4];
}

PSRectangle::PSRectangle(const Text_t* n, const Text_t* t) :
  ParaSurf(n,t)
{
  _init();
}

PSRectangle::~PSRectangle()
{
  delete [] mEdgePlanes;
}

/**************************************************************************/

void PSRectangle::SetupEdgePlanes()
{
  // !!! should be called from AdEnlightenment

  mEdgePlanes[0].Set( 0, -1, 0,  mMinY);
  mEdgePlanes[1].Set(-1,  0, 0,  mMinX);
  mEdgePlanes[2].Set( 0,  1, 0, -mMaxY);
  mEdgePlanes[3].Set( 1,  0, 0, -mMaxX);
}

/**************************************************************************/

Float_t PSRectangle::surface()
{
  return (mMaxX - mMinX) * (mMaxY - mMinY);
}

/******************************************************************************/

void PSRectangle::origin_fgh(Float_t* f)
{
  f[0] = mMinX + 0.5f*(mMaxX - mMinX);
  f[1] = mMinY + 0.5f*(mMaxY - mMinY);
  f[2] = 0;
}

void PSRectangle::origin_pos(Float_t* x)
{
  x[0] = mMinX + 0.5f*(mMaxX - mMinX);
  x[1] = mMinY + 0.5f*(mMaxY - mMinY);
  x[2] = 0;
}

/**************************************************************************/

void PSRectangle::pos2fgh(const Float_t* x, Float_t* f)
{
  f[0] = x[0];
  f[1] = x[1];
  f[2] = x[2];
}

void PSRectangle::fgh2pos(const Float_t* f, Float_t* x)
{
  x[0] = f[0];
  x[1] = f[1];
  x[2] = f[2];
}

void PSRectangle::fgh2fdir(const Float_t* f, Float_t* d)
{
  d[0] = 1;
  d[1] = 0;
  d[2] = 0;
}

void PSRectangle::fgh2gdir(const Float_t* f, Float_t* d)
{
  d[0] = 0;
  d[1] = 1;
  d[2] = 0;
}

void PSRectangle::fgh2hdir(const Float_t* f, Float_t* d)
{
  d[0] = 0;
  d[1] = 0;
  d[2] = 1;
}

/**************************************************************************/

void PSRectangle::pos2hdir(const Float_t* x, Float_t* d)
{
  // Return 'up' direction.

  d[0] = 0;
  d[1] = 0;
  d[2] = 1;
}
 
Float_t PSRectangle::pos2hray(const Float_t* x, Opcode::Ray& r)
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

void PSRectangle::random_fgh(TRandom& rnd, Float_t* f)
{
  f[0] = rnd.Uniform(mMinY, mMaxY);
  f[1] = rnd.Uniform(mMinY, mMaxY);
  f[2] = 0;
}

void PSRectangle::random_pos(TRandom& rnd, Float_t* x)
{
  x[0] = rnd.Uniform(mMinX, mMaxX);
  x[1] = rnd.Uniform(mMinY, mMaxY);
  x[2] = 0;
}

/**************************************************************************/

void PSRectangle::wrap(Float_t* x, Int_t plane, Float_t dist)
{
  using namespace Opcode;

  Point& pos = *(Point*)x;

  Int_t opp_plane = (plane + 2) % 4;
  Plane& P = mEdgePlanes[plane];
  Plane& O = mEdgePlanes[opp_plane];

  Float_t opp_dist = O.Distance(pos);
  pos += dist * P.n - opp_dist * O.n;
}

/**************************************************************************/

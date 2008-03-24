// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PSTorus
//
// Parametric toroidal surface.
// bInside ~ false: 'world' on outer surface
// bInside ~ true:  'world' on inner surface
//
//   | Outside             | Inside
//-----------------------------------------------
// f | phi   [-pi, pi ] | phi   [inverted]
// g | theta [-pi, pi ] | theta [same    ]
// h | dr    [-mR, inf] | dr    [mR, -inf]
//
// f,g,h vectors form a right-handed ortho-normal base.
// 'outside' -> 'inside' : { f -> -f, h -> -h. }


#include "PSTorus.h"
#include "PSTorus.c7"

#include <Opcode/Opcode.h>

#include <TMath.h>
#include <TRandom.h>

ClassImp(PSTorus);

/**************************************************************************/

void PSTorus::_init()
{
  bInside = false;
  mRM = 1;
  mRm = 0.5;
}

namespace {
inline float norm3(float x, float y, float z)
{ return sqrtf(x*x + y*y + z*z); }
}

/**************************************************************************/

Float_t PSTorus::surface()
{
  return 4.0f*sPi*sPi*mRM*mRm;
}

/******************************************************************************/

void PSTorus::pos2fgh(const Float_t* x, Float_t* f)
{
  f[0] = atan2f(x[1], x[0]);
  f[1] = atan2f(x[2], sqrtf(x[0]*x[0] + x[1]*x[1]) - mRM);
  f[2] = norm3(x[0] - mRM*cosf(f[0]), x[1] - mRM*sinf(f[0]), x[2]) - mRm;
  if (bInside) { f[0] = -f[0]; f[2] = -f[2]; }
}

void PSTorus::fgh2pos(const Float_t* f, Float_t* x)
{
  const Float_t rphi = bInside ? mRm - f[2] : mRm + f[2];
  const Float_t rxy  = mRM + rphi*cosf(f[1]);

  x[0] = rxy  * cosf(f[0]); // cos is even
  x[1] = rxy  * sinf(f[0]); if (bInside) x[1] = -x[1];
  x[2] = rphi * sinf(f[1]);
}

void PSTorus::fgh2fdir(const Float_t* f, Float_t* d)
{
  d[0] = -sinf(f[0]);
  d[1] =  cosf(f[0]);
  d[2] =  0;
  if (bInside) {
    // d[0] = -d[0]; already negated by phi inversion and oddness of sin
    d[1] = -d[1];
    // d[2] = -d[2]; zero
  }
}

void PSTorus::fgh2gdir(const Float_t* f, Float_t* d)
{
  const Float_t st = -sinf(f[1]);

  d[0] = st * cosf(f[0]);
  d[1] = st * sinf(f[0]); if (bInside) d[1] = -d[1];
  d[2] = cosf(f[1]);
}

void PSTorus::fgh2hdir(const Float_t* f, Float_t* d)
{
  const Float_t ct = cosf(f[1]);

  d[0] = ct * cosf(f[0]);
  d[1] = ct * sinf(f[0]);
  d[2] = sinf(f[1]);
  if (bInside) {
    d[0] = -d[0];
    // d[1] = -d[1]; already negated by phi inversion and oddness of sin
    d[2] = -d[2];
  }
}

/**************************************************************************/

void PSTorus::pos2hdir(const Float_t* x, Float_t* d)
{
  // Return 'up' direction.

  const Float_t p  = atan2f(x[1], x[0]);
  const Float_t t  = atan2f(x[2], sqrtf(x[0]*x[0] + x[1]*x[1]) - mRM);
  const Float_t ct = cosf(t);

  d[0] = ct*cosf(p);
  d[1] = ct*sinf(p);
  d[2] = sinf(t);
  if (bInside) {
    d[0] = -d[0];
    d[1] = -d[1];
    d[2] = -d[2];
  }
}
 
Float_t PSTorus::pos2hray(const Float_t* x, Opcode::Ray& r)
{
  // Setup ray r for given postition x so that the ray origin is above
  // the surface and its direction/lenght ascertain the surface will
  // be intersected.
  // Returns distance the ray-origin was shifted from initial pos.

  const Float_t p  = atan2f(x[1], x[0]);
  const Float_t t  = atan2f(x[2], sqrtf(x[0]*x[0] + x[1]*x[1]) - mRM);
  const Float_t ct = cosf(t);
  const Float_t sp = sinf(p), cp = cosf(p);

  Float_t dr = norm3(x[0] - mRM*cp, x[1] - mRM*sp, x[2]) - mRm;
  r.mDir.Set(ct*cp, ct*sp, sinf(t));
  if (bInside) {
    r.mDir.Neg();
    dr = -dr;
  }
  Float_t dist = mMaxH - dr + mEpsilon;

  r.mOrig.Set(x);
  r.mOrig.TMac(r.mDir, dist);
  r.mDir.Neg();
  /*
  printf("pos=%.2f,%.2f,%.2f; orig=%.2f,%.2f,%.2f; dir=%.2f,%.2f,%.2f; dist=%.2f\n",
         x[0],      x[1],      x[2],
         r.mOrig.x, r.mOrig.y, r.mOrig.z,
         r.mDir.x,  r.mDir.y,  r.mDir.z,
         dist);
  */
  return dist;
}

/**************************************************************************/

void PSTorus::sub_fgh(Float_t* a, Float_t* b, Float_t* delta)
{
  // Subtract fgh values, taking care of U(1) variables (like angles).
  // Here this is done for phi and theta.

  delta[0] = U1Sub(a[0], b[0]);
  delta[1] = U1Sub(a[1], b[1]);
  delta[2] = a[2] - b[2];
}

void PSTorus::regularize_fg(Float_t* f)
{
  // Put fg values into regular intervals.

  if (f[0] > sPi)       f[0] -= sTwoPi;
  else if (f[0] < -sPi) f[0] += sTwoPi;

  if (f[1] > sPi)       f[1] -= sTwoPi;
  else if (f[1] < -sPi) f[1] += sTwoPi;
}

void PSTorus::random_fgh(TRandom& rnd, Float_t* f)
{
  const Float_t k  = mRm/mRM;
  const Float_t P  = rnd.Uniform(-TMath::Pi(), TMath::Pi());
  Float_t t = P, d = 0;
  do {
    d  = (P - t - k*sinf(t)) / (1 + k*cosf(t));
    t += d;
  } while (fabsf(d) > 1e-6);

  f[0] = rnd.Uniform(-TMath::Pi(), TMath::Pi());
  f[1] = t;
  f[2] = 0;
}

/**************************************************************************/

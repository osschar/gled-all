// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PSSphere.h"
#include "PSSphere.c7"
#include <Stones/GravData.h>

#include <Opcode/Opcode.h>

#include <TMath.h>
#include <TRandom.h>

//==============================================================================
// PSSphere
//==============================================================================

//__________________________________________________________________________
//
// Parametric sphere surface.
// bInside ~ false: 'world' on outer surface, like a planet.
// bInside ~ true:  'world' on inner surface, like Dyson sphere.
//
//   | Outside             | Inside
//-----------------------------------------------
// f | phi   [  -pi, pi  ] | phi   [inverted]
// g | theta [-pi/2, pi/2] | theta [same    ]
// h | dr    [  -mR, inf ] | dr    [mR, -inf]
//
// f,g,h vectors form a right-handed ortho-normal base.
// 'outside' -> 'inside' : { f -> -f, h -> -h. }

ClassImp(PSSphere);

/**************************************************************************/

void PSSphere::_init()
{
  bInside = false;
  mR = 1;
}

/**************************************************************************/

void PSSphere::FindMinMaxFGH(TriMesh* mesh)
{
  PARENT_GLASS::FindMinMaxH(mesh);

  mMinF = -sPi;     mMaxF = sPi;
  mMinG = -sPiHalf; mMaxG = sPiHalf;
}

/**************************************************************************/

Float_t PSSphere::Surface()
{
  return 4.0f*sPi*mR*mR;
}

/******************************************************************************/

void PSSphere::pos2fgh(const Float_t* x, Float_t* f)
{
  const Float_t rxysq = x[0]*x[0] + x[1]*x[1];
  const Float_t r     = sqrtf(rxysq + x[2]*x[2]);

  f[0] = atan2f(x[1], x[0]);
  f[1] = atan2f(x[2], sqrt(rxysq));
  f[2] = r - mR;
  if (bInside) { f[0] = -f[0]; f[2] = -f[2]; }
}

void PSSphere::fgh2pos(const Float_t* f, Float_t* x)
{
  const Float_t r   = bInside ? mR - f[2] : mR + f[2];
  const Float_t rct = r*cosf(f[1]);

  x[0] = rct * cosf(f[0]); // cos is even
  x[1] = rct * sinf(f[0]); if (bInside) x[1] = -x[1];
  x[2] =   r * sinf(f[1]);
}

void PSSphere::fgh2fdir(const Float_t* f, Float_t* d)
{
  d[0] = -sinf(f[0]);
  d[1] =  cosf(f[0]);
  d[2] =  0;
  if (bInside) {
    // d[0] = -d[0]; already negated by phi inversion and oddness of sin
    d[1] = -d[1];
    d[2] = -d[2];
  }
}

void PSSphere::fgh2gdir(const Float_t* f, Float_t* d)
{
  const Float_t st = -sinf(f[1]);

  d[0] = st * cosf(f[0]);
  d[1] = st * sinf(f[0]); if (bInside) d[1] = -d[1];
  d[2] = cosf(f[1]);
}

void PSSphere::fgh2hdir(const Float_t* f, Float_t* d)
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

void PSSphere::pos2hdir(const Float_t* x, Float_t* d)
{
  // Return 'up' direction.

  const Float_t a = bInside ? -1.0f : 1.0f;
  Float_t q = sqrtf(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
  if (q != 0) {
    q = a / q;
    d[0] = q * x[0];  d[1] = q * x[1];  d[2] = q * x[2];
  } else {
    d[0] = 0;  d[1] = 0;  d[2] = a;
  }
}

Float_t PSSphere::pos2hray(const Float_t* x, Opcode::Ray& r)
{
  // Setup ray r for given postition x so that the ray origin is above
  // the surface and its direction/lenght ascertain the surface will
  // be intersected.
  // Returns distance the ray-origin was shifted from initial pos.

  const Float_t a = bInside ? -1.0f : 1.0f;
  Float_t q    = sqrtf(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
  Float_t h    = bInside ? mR - q : q - mR;
  Float_t dist = mMaxH - h + mEpsilon;

  // printf("a=%f; q=%f; dr=%f; dist=%f;\n", a, q, dr, dist);

  if (q != 0) {
    q = a / q;
    r.mDir.Set(q * x[0], q * x[1], q * x[2]);
  } else {
    r.mDir.Set(0, 0, a);
  }
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

void PSSphere::pos2grav(const Float_t* x, GravData& gd)
{
  gd.fPos[0] = x[0]; gd.fPos[1] = x[1]; gd.fPos[2] = x[2];

  fill_spherical_grav(mGravAtSurface, mR, bInside, x, gd);

  /*
  const Float_t rr = x[0]*x[0] + x[1]*x[1] + x[2]*x[2];
  const Float_t r  = sqrtf(rr);

  if (r > mR) {
    gd.fMag   = mGravAtSurface * mR * mR / rr;
    gd.fLDer  = gd.fTDer = gd.fMag / r;
    gd.fLDer *= 2.0f;    
  } else {
    gd.fLDer = gd.fTDer = mGravAtSurface / mR;
    gd.fMag  = gd.fLDer * r;
  }

  Float_t a;
  if (bInside) {
    gd.fH = mR - r;
    a     = 1.0f;
  } else {
    gd.fH =  r - mR;
    a     = -1.0f;
  }

  if (r != 0) {
    const Float_t q = a / r;
    gd.fDir[0] = q * x[0];  gd.fDir[1] = q * x[1];  gd.fDir[2] = q * x[2];
  } else {
    gd.fDir[0] = gd.fDir[1] = gd.fDir[2] = 0;
  }
  */
}

/**************************************************************************/

void PSSphere::sub_fgh(Float_t* a, Float_t* b, Float_t* delta)
{
  // Subtract fgh values, taking care of U(1) variables (like angles).
  // Here this is done for phi.

  delta[0] = U1Sub(a[0], b[0]);
  delta[1] = a[1] - b[1];
  delta[2] = a[2] - b[2];
}

void PSSphere::regularize_fg(Float_t* f)
{
  // Put fg values into regular intervals.

  if (f[0] > sPi)       f[0] -= sTwoPi;
  else if (f[0] < -sPi) f[0] += sTwoPi;

  if (f[1] < -sPiHalf)     f[1] = -sPiHalf;
  else if (f[1] > sPiHalf) f[1] =  sPiHalf;
}

void PSSphere::random_fgh(TRandom& rnd, Float_t* f)
{
  f[0] = rnd.Uniform(-TMath::Pi(), TMath::Pi());
  f[1] = TMath::ASin(rnd.Uniform(-1, 1));
  f[2] = 0;
}

void PSSphere::random_pos(TRandom& rnd, Float_t* x)
{
  const Float_t p   = rnd.Uniform(-TMath::Pi(), TMath::Pi());
  const Float_t t   = TMath::ASin(rnd.Uniform(-1, 1));
  const Float_t rct = mR*cosf(t);

  x[0] = rct * cosf(p);
  x[1] = rct * sinf(p);
  x[2] =  mR * sinf(t);
}

/**************************************************************************/

void PSSphere::fill_spherical_grav(Float_t g0, Float_t R, Bool_t inside,
                                   const Float_t* x, GravData& gd)
{
  // Fills passed GravData as determined by other parameters.
  // GravData::fPos is not set.

  const Float_t rr = x[0]*x[0] + x[1]*x[1] + x[2]*x[2];
  const Float_t r  = sqrtf(rr);

  if (r > R) {
    gd.fMag   = g0 * R * R / rr;
    gd.fLDer  = gd.fTDer = gd.fMag / r;
    gd.fLDer *= 2.0f;    
  } else {
    gd.fLDer = gd.fTDer = g0 / R;
    gd.fMag  = gd.fLDer * r;
  }

  Float_t a;
  if (inside) {
    gd.fH = R - r;
    a     = 1.0f;
  } else {
    gd.fH =  r - R;
    a     = -1.0f;
  }

  if (r != 0) {
    const Float_t q = a / r;
    gd.fDir[0] = q * x[0];  gd.fDir[1] = q * x[1];  gd.fDir[2] = q * x[2];
  } else {
    gd.fDir[0] = gd.fDir[1] = gd.fDir[2] = 0;
  }

  // Copy down direction.
  gd.fDown[0] = gd.fDir[0]; gd.fDown[1] = gd.fDir[1]; gd.fDown[2] = gd.fDir[2];
}

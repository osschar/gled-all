// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Dynamico
//
// We have those reference data-members mV and mW here ... they are needed so
// that we can have WGlValuator widgets that pick up all the stuff from the
// dictionaries. 

#include "Dynamico.h"
#include "Dynamico.c7"

#include "Statico.h"
#include "Tringula.h"
#include "ParaSurf.h"

#include <Stones/TringTvor.h>

#include <TMath.h>

ClassImp(Dynamico);

/**************************************************************************/

void Dynamico::_init()
{
  mLastTransPtr = &mLastTrans;
  mLastAABBPtr  = &mLastAABB;

  mSafety = mExtraStep = 0;

  mOPCRCCache = Opcode::OPC_INVALID_ID;
}

Dynamico:: Dynamico(const Text_t* n, const Text_t* t) :
  Extendio(n,t),
  mV(mVVec.x),
  mW(mWVec.x)
{
  _init();
}

//==============================================================================

void Dynamico::SetTringula(Tringula* tring)
{
  // Set tringula to which the extendio is attached.
  // Sub-classes override this to reinitialize cached data.

  PARENT_GLASS::SetTringula(tring);

  mTringula->GetParaSurf()->pos2grav(mTrans.ArrT(), mGrav);
  mOPCRCCache = Opcode::OPC_INVALID_ID;
}

//==============================================================================

void Dynamico::update_grav_safeties(Float_t vl, Float_t vt)
{
  // Calculate time and distance over which we can keep this gravity data.
  // There hard-coded limits here!
  //
  // Safe time:
  // ----------
  // For non-accelerated motion this is:
  //   t = (6 epsilon / g' v)^1/3
  // Where epsilon is precision in position and "g' v" is the product of
  // gravitational acceleration derivative and corresponding velocity.
  // Thus we search for maximal one.
  // Epsilon is set to 0.1m - this could be more, probably.
  //
  // Problem is, if the unit accelerates during the period. This
  // is not handled at all. Maybe could take current / max accel
  // into account - or use distance instead of time!
  //
  // Also - some limits need to be imposed.
  // 1. Maximal time is set to max_time. This is somewhat arbitrary.
  //    This way we enforce re-checking of the field periodically.
  // 2. Minimal time we bother to calculate is min_time which should be
  //    about the order of dt.
  //
  // Safe distance:
  // --------------
  // Maximum distance - where gravity changes for more then certain
  // value eta:
  //   L = eta / g'

  static const Float_t epsilon    = 6 * 0.1;  // Precision of position.
  static const Float_t max_time   = 10;
  static const Float_t min_time   = 0.01;
  static const Float_t lim_gv_low = epsilon / (max_time*max_time*max_time);
  static const Float_t lim_gv_hi  = epsilon / (min_time*min_time*min_time);

  static const Float_t eta        = 0.1;
  static const Float_t max_dist   = 1000;
  static const Float_t lim_g_low  = eta / max_dist;

  Float_t max_gv = TMath::Max(mGrav.fLDer * vl, mGrav.fTDer * vt);

  if (max_gv < lim_gv_low)
    mGrav.fSafeTime = max_time;
  else if (max_gv > lim_gv_hi)
    mGrav.fSafeTime = 0;
  else
    mGrav.fSafeTime = powf(epsilon / max_gv, 0.3333333333f);

  Float_t max_g = TMath::Max(mGrav.fLDer, mGrav.fTDer);

  if (max_g < lim_g_low)
    mGrav.fSafeDistance = max_dist;
  else
    mGrav.fSafeDistance = eta / max_g;
}


/**************************************************************************/

bool Dynamico::handle_collision(Dynamico            * dyno,
                                Statico             * stato,
                                Opcode::RayCollider & ray_collider,
                                Opcode::Ray         & ray,
                                Opcode::Point       & com_dyno,
                                CollisionSegments   & segments)
{
  // Handle collision between dynamico and statico.
  //
  // If com_dyno is inside statico, let it go, we're too late (or it
  // was inside to start with).
  // If collision-vertex is in front of com_dyno bounce off with change of
  // direction, otherwise just move away from collision-vertex.
  //
  // Input:
  //   com_dyno center-of-mass of dynamico
  // Return:    false if ray collision fails.

  static const Exc_t _eh("Dynamico::handle_collision ");

  if (ray_collider.Collide(ray, *stato->get_opc_model(), stato->RefTrans()))
  {
    using namespace Opcode;

    Opcode::CollisionFaces& CF = * ray_collider.GetDestination();

    // Do fixing only when nfaces is even, i.e., dyno's com_dyno is outside.
    if (CF.GetNbFaces() % 2 == 0)
    {
      // center-vertex of collision segments.
      const Point& cv = segments.RefCenter();

      Point com2cv(cv); com2cv -= com_dyno;

      HTransF& t_dyno = dyno->mTrans;

      Point vel;      t_dyno.RotateVec3(dyno->mVVec, vel);
      Point fwd(vel); fwd.Normalize();

      if ((vel | com2cv) > 0 && segments.size() > 1)
      {
        // CV is in front - turn to be parallel with collision plane
        // or a bit further as plane normal is not projected into xy
        // plane of dyno.
        // If segments.size() == 1 => plane is not defined (happens rarely).

        Plane P(segments[0].mP0, segments[0].mP1, cv);
        // turn to be parallel with collision plane or a bit further
        // as plane normal is not projected into xy plane of dyno.
        Float_t dp = fwd | P.n;
        if (dp < 0) P.n.Neg();
        Point cross = P.n ^ fwd;
        Float_t  cp = 0.5f * cross.Magnitude();
        if ((cross | *(Point*) t_dyno.PtrBaseVec(3)) < 0)
          t_dyno.RotateLF(2, 1, TMath::PiOver2() - TMath::ASin(cp));
        else
          t_dyno.RotateLF(1, 2, TMath::PiOver2() - TMath::ASin(cp));
      }
      else
      {
        // CV is behind - move away from it.
        // !!! 0.1 should be replaced with a ray from cv back towards com_dyno.
        com2cv *= -0.1f;
        t_dyno.Move3PF(com2cv.x, com2cv.y, com2cv.z);

        // When a dyno is moved in collision, safeties need to be updated, too.
        dyno->mExtraStep += com2cv.Magnitude();
      }

      // Check for nan's.
      // assert(!isnan(t_dyno[12]) && !isnan(t_dyno[0]) && !isnan(t_dyno[4]) && !isnan(t_dyno[8]));
    }
    return true;
  }
  else
  {
    return false;
  }
}

bool Dynamico::handle_collision(Dynamico          * dyno0,
                                Dynamico          * dyno1,
                                Opcode::Point     & up_dir,
                                CollisionSegments & segments)
{
  // Handle collision of two dynamicos.
  //
  // If they are coming together, they are bounced off (momentum
  // conservation in tangential plane), otherwise they are just
  // separated for a fixed amount.
  //
  // Returns:   true, for the time being.

  using namespace Opcode;

  HTransF& t0 = dyno0->ref_trans();
  HTransF& t1 = dyno1->ref_trans();

  // Centers of mass
  Point com0; t0.MultiplyVec3(dyno0->GetMesh()->RefCOM(), 1.0f, com0);
  Point com1; t1.MultiplyVec3(dyno1->GetMesh()->RefCOM(), 1.0f, com1);
  Point dcom; dcom.Sub(com1, com0);
  // Diff of centers of mass in tangetial plane
  Point dcomp(dcom); dcomp.TMsc(up_dir, up_dir | dcom);

  // Velocities
  // Point v0(t0.PtrBaseVec(1)); v0 *= dyno0->mV;
  // Point v1(t1.PtrBaseVec(1)); v1 *= dyno1->mV;
  Point v0; t0.RotateVec3(dyno0->mVVec, v0);
  Point v1; t0.RotateVec3(dyno1->mVVec, v1);
  // Velocities in tangential plane
  Point v0p(v0); v0p.TMsc(up_dir, up_dir | v0);
  Point v1p(v1); v1p.TMsc(up_dir, up_dir | v1);

  Point dv; dv.Sub(v1p, v0p);

  if ((dcomp | dv) < 0)
  {
    // they are moving closer, bounce them off

    Point dv_dir = dv; dv_dir.Normalize();

    // Do the calculation in coord system where dyno-0 is at rest,
    // dyno-1 is coming in with v1-v0.
    Float_t w1 = dv.Magnitude();
    Float_t m0 = dyno0->GetMesh()->GetM();
    Float_t m1 = dyno1->GetMesh()->GetM();
    Float_t P    = w1 * m1; // momentum
    Float_t TwoT = w1 * P;  // two-times kinetic energy
    Float_t a = m1 * m1 + m0 * m1;
    Float_t b = -2.0f * P * m1;
    Float_t c = P * P - TwoT * m0;

    Float_t w1p = -0.5f * (b + sqrtf(b*b - 4.0f*a*c)) / a;
    Float_t w0p = (P - m1*w1p) / m0;

    // XXXX New velocities and transformations, calculated and assigned below,
    // put all velocity in the x/forward direction. In principle could also assign
    // slide-velocities, but these are not decayed in TimeTick() so the thing
    // would go sour soon.
    // Need an epsilon for min. new velocity magnitude ... see hardcoded 1e-5 below.

    v0.TMac(dv_dir, w0p);
    Float_t vm0 = v0.Magnitude();
    if (vm0 > 1e-5)
    {
      v0 *= 1.0f / vm0;
      t0.SetBaseVec(1, v0);
      t0.OrtoNorm3Column(1, 3);
      t0.SetBaseVecViaCross(2);
      dyno0->mVVec.Set(vm0, 0, 0);
    }

    v1.TMac(dv_dir, w1p - w1);
    Float_t vm1 = v1.Magnitude();
    if (vm1 > 1e-5)
    {
      v1 *= 1.0f / vm1;
      t1.SetBaseVec(1, v1);
      t1.OrtoNorm3Column(1, 3);
      t1.SetBaseVecViaCross(2);
      dyno1->mVVec.Set(vm1, 0, 0);
    }
  } else {
    // they are moving apart, just separate them some more
    dcomp *= 0.1f;
    t0.Move3PF(-dcomp.x, -dcomp.y, -dcomp.z);
    t1.Move3PF( dcomp.x,  dcomp.y,  dcomp.z);

    // When a dyno is moved in collision, safeties need to be updated, too.
    const Float_t extra_step = dcomp.Magnitude();
    dyno0->mExtraStep += extra_step;
    dyno1->mExtraStep += extra_step;
  }

  // Check for nan's.
  // assert(!isnan(t0[12]) && !isnan(t0[0]) && !isnan(t0[4]) && !isnan(t0[8]));
  // assert(!isnan(t1[12]) && !isnan(t1[0]) && !isnan(t1[4]) && !isnan(t1[8]));

  return true;
}

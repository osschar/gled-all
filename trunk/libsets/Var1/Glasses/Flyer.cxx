// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Flyer.h"
#include "Flyer.c7"
#include <Stones/GravData.h>

#include "Tringula.h"
#include "ParaSurf.h"

// Flyer

//______________________________________________________________________________
//
//

ClassImp(Flyer);

//==============================================================================

void Flyer::_init()
{
  mHeight       = 0;    // Height above current surface - there always is one for now.

  mGravHChange  = 0;    // Integrated up/down path between gravity changes.
  bGravFixUpDir = true; // Rotate after change in gravity direction.
}

Flyer::Flyer(const Text_t* n, const Text_t* t) :
  Dynamico(n, t)
{
  _init();
}

Flyer::~Flyer()
{}

//==============================================================================

void Flyer::TimeTick(Double_t t, Double_t dt)
{
  // Move flyer in for given dt.
  // 1. Move with current velocity.
  // 2. Perform rotations.
  // 3. Check boundaries
  // 4. Check gravity change
  // 5. Calculate velocities for the next step.

  static const Exc_t _eh("Flyer::TimeTick ");

  if (bParked)
    return;

  using namespace Opcode;

  RayCollider    RC;
  RC.SetFirstContact(true);
  RC.SetTemporalCoherence(true);
  CollisionFaces CF;
  RC.SetDestination(&CF);

  Opcode::Point velocity; // Velocity in master frame.
  Float_t       velocity_mag2, velocity_mag, step_length;

  mTrans.RotateVec3(mVVec, velocity);
  velocity_mag2 = velocity.SquareMagnitude();
  velocity_mag  = sqrtf(velocity_mag2);

  step_length   = velocity_mag * dt;

  // XXXYY Here I was using gravity direction for calculation of
  // height correction - but this didn't work very well on torus as
  // down (towards closes ring) and gravity are not parallel.
  // With current state of flight mechanics I have to use down direction
  // to properly apply the correction.
  //
  // This doesn't conserve the potential energy ... so revisit this once
  // better control for flight parameters is available (auto-steering, corrections).
  //
  // In particular, the flyers could internally strive towards straight flight. As
  // we have the gravity direction, we can calculate which angular velocities
  // (in local frame) need to be tweaked for stabilization of flight. This should
  // be damped critically ... or even more.
  //
  // There are several spots in this function marked with XXXYY.

  const Float_t dh = (mGrav.Down() | velocity) * dt;
  // XXXYY const Float_t dh = (mGrav.Dir() | velocity) * dt;
  mHeight      -= dh;
  mGravHChange -= dh;

  // Use velocity - is needed afterwards, too.
  // [But can be changed by CheckBoundaries()!]
  // mTrans.Move3LF(mVVec.x * dt, mVVec.y * dt, mVVec.z * dt);
  mTrans.Move3PF(velocity.x * dt, velocity.y * dt, velocity.z * dt);

  mTrans.RotateLF(1, 2, dt*mWVec.x);
  mTrans.RotateLF(2, 3, dt*mWVec.y);
  mTrans.RotateLF(3, 1, dt*mWVec.z);

  mSafety -= step_length;
  if (mSafety < 0)
  {
    // Check boundaries - this can result in tringula switch.
    Bool_t trans_changed = mTringula->CheckBoundaries(this, mSafety);

    if (trans_changed)
    {
      // Recalculate velocity
      mTrans.RotateVec3(mVVec, velocity);
      velocity_mag2 = velocity.SquareMagnitude();
      velocity_mag  = sqrtf(velocity_mag2);

      // Invalidate position dependant caches.
      // GravData should be fine as it is supposedly smooth.
      // Terrain safety ok, too, we should check neighbouring meshes
      // when calculating it.
    }
  }

  Opcode::Point& pos = * (Opcode::Point*) ref_trans().ArrT();
  // Opcode::Point& up  = * (Opcode::Point*) ref_trans().ArrZ();

  if (mGrav.DecaySafeties(dt, step_length))
  {
    // Cache values from old grav-data needed for position fixes.
    Float_t       assumed_h = mGrav.fH + mGravHChange;
    // XXXYY Opcode::Point old_grav_dir(mGrav.fDir);
    Opcode::Point old_grav_dir(mGrav.fDown);

    mTringula->GetParaSurf()->pos2grav(pos, mGrav);

    // Calculate time we can keep this gravity data.
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

    static const Float_t epsilon    = 6 * 0.1;  // Precision of position.
    static const Float_t max_time   = 10;
    static const Float_t min_time   = 0.01;
    static const Float_t lim_gv_low = epsilon / (max_time*max_time*max_time);
    static const Float_t lim_gv_hi  = epsilon / (min_time*min_time*min_time);

    // But also, have a maximum distance - where gravity changes for
    // more then certain value eta:
    //   L = eta / g'

    static const Float_t eta        = 0.1;
    static const Float_t max_dist   = 1000;
    static const Float_t lim_g_low  = eta / max_dist;

    Float_t vl = mGrav.Dir() | velocity;  if (vl < 0) vl = -vl;
    Float_t vt = sqrtf(velocity_mag2 - vl*vl);

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

    // Fix height difference due to change in gravity direction.
    // This assures we keep proper potential energy.

    Float_t dh = mGrav.fH - assumed_h;
    // XXXYY mTrans.Move3PF(mGrav.fDir[0] * dh, mGrav.fDir[1] * dh, mGrav.fDir[2] * dh);
    mTrans.Move3PF(mGrav.fDown[0] * dh, mGrav.fDown[1] * dh, mGrav.fDown[2] * dh);

    mHeight      = assumed_h;
    mGrav.fH     = assumed_h;
    mGravHChange = 0;

    // Optionally rotate for difference in gravity direction.

    if (bGravFixUpDir)
    {
      Opcode::Matrix3x3 m;
      // XXXYY m.FromTo(old_grav_dir, mGrav.Dir());
      m.FromTo(old_grav_dir, mGrav.Down());
      mTrans.MultLeft3x3(m);
    }

    // Testing printout.
    if (bSelected)
    {
      mGrav.Print();
      printf("  v_mag=%f, vl=%f, vt=%f, max_gv=%f, t_safe=%f, d_safe=%f (d/v)=%f\n",
             velocity_mag, vl, vt, max_gv, mGrav.fSafeTime, mGrav.fSafeDistance,
             mGrav.fSafeDistance / velocity_mag);
    }
  }
}

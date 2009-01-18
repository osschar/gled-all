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
      // GravData should be fine as it is supposedly smooth. But anyway,
      // it is reset via SetTringula() which leaves safeties at 0.
      // Terrain safety ok, too, we should check neighbouring meshes
      // when calculating it anyway.
    }
  }

  if (mGrav.DecaySafeties(dt, step_length))
  {
    // Cache values from old grav-data needed for position fixes.
    Float_t       assumed_h = mGrav.fH + mGravHChange;
    // XXXYY Opcode::Point old_grav_dir(mGrav.fDir);
    Opcode::Point old_grav_dir(mGrav.fDown);

    mTringula->GetParaSurf()->pos2grav(ref_trans().ArrT(), mGrav);

    Float_t vl = mGrav.Dir() | velocity;  if (vl < 0) vl = -vl;
    Float_t vt = sqrtf(velocity_mag2 - vl*vl);

    update_grav_safeties(vl, vt);

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
      printf("  v_mag=%f, vl=%f, vt=%f, t_safe=%f, d_safe=%f (d/v)=%f\n",
             velocity_mag, vl, vt, mGrav.fSafeTime, mGrav.fSafeDistance,
             mGrav.fSafeDistance / velocity_mag);
    }
  }
}

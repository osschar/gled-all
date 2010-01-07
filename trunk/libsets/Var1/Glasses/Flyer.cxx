// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Flyer.h"
#include "Flyer.c7"

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

  mTerrainSafety      = 0;
  mTerrainProbeRadius = 0;
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

  Opcode::Point velocity; // Velocity in master frame.
  Float_t       velocity_mag2, velocity_mag, step_length;

calculate_velocity:
  mTrans.RotateVec3(mVVec, velocity);
  velocity_mag2 = velocity.SquareMagnitude();
  velocity_mag  = sqrtf(velocity_mag2);

  step_length   = velocity_mag * dt + mExtraStep;
  mExtraStep    = 0;

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

  const Float_t dh = - (mGrav.Down() | velocity) * dt;
  // XXXYY const Float_t dh = (mGrav.Dir() | velocity) * dt;
  if (mHeight + dh > mTringula->GetMaxFlyerH())
  {
    // This is another steaming hack - to prevent flyers form exceeding max-h.
    // Subtract the vertical velocity and redo the calculation.

    // We can't do much if it is already beyond max-height.
    if (mHeight < mTringula->GetMaxFlyerH())
    {
      printf("Fixing speed for %s, hp=%f, mh=%f\n", GetName(), mHeight + dh, mTringula->GetMaxFlyerH());

      velocity.TMac(mGrav.Down(), dh / dt);
      mTrans.RotateBackVec3(velocity, mVVec);

      goto calculate_velocity;
    }
  }
  mHeight      += dh;
  mGravHChange += dh;

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

    Float_t fdh = mGrav.fH - assumed_h;
    // XXXYY mTrans.Move3PF(mGrav.fDir[0] * fdh, mGrav.fDir[1] * fdh, mGrav.fDir[2] * fdh);
    mTrans.Move3PF(mGrav.fDown[0] * fdh, mGrav.fDown[1] * fdh, mGrav.fDown[2] * fdh);

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
    if (mDebugBits & DB_GravData)
    {
      mGrav.Print();
      printf("  v_mag=%f, vl=%f, vt=%f, t_safe=%f, d_safe=%f (d/v)=%f\n",
             velocity_mag, vl, vt, mGrav.fSafeTime, mGrav.fSafeDistance,
             mGrav.fSafeDistance / velocity_mag);
    }
  }

  mTerrainSafety -= step_length;
  if (mTerrainSafety < 0)
  {
    static const Float_t probe_inc_fac = 1.414;
    static const Float_t probe_dec_fac = 0.707;

    Float_t min_r = mMesh->GetTTvor()->GetMaxVertexDistance();

    Opcode::SphereCache    SCache;
    Opcode::SphereCollider SC;
    SC.SetFirstContact(true);

    Opcode::Sphere S(ref_pos(), 0);

    while (mTerrainProbeRadius >= min_r)
    {
      S.SetRadius(mTerrainProbeRadius);

      bool cs = SC.Collide(SCache, S, *mTringula->GetMesh()->GetOPCModel());
      if (cs)
      {
        if (SC.GetContactStatus())
        {
          mTerrainProbeRadius *= probe_dec_fac;
        }
        else
        {
          mTerrainSafety = mTerrainProbeRadius;
          mTerrainProbeRadius *= probe_inc_fac;
          break;
        }
      }
      else
      {
        printf("%s something wrong in sphere-terrain collision test.\n",
               _eh.Data());
      }
    }

    if (mTerrainProbeRadius < min_r)
    {
      CollisionSegments segments;
      Int_t ns = collide_with_tringula(segments);
      if (ns > 0)
      {
        // Do this trivially ... if it is coming closer to the collision
        // point, turn it around, otherwise do nothing.

        Opcode::Point dr(segments.RefCenter());
        dr.Sub(ref_pos());
        if ((dr | velocity) > 0)
        {
          mTrans.RotateLF(1, 2, TMath::Pi());
        }

        // If extra move of flyer is done, also add it to mExtraStep.
      }
      mTerrainSafety      = 0;
      mTerrainProbeRadius = min_r;
    }
  }
}

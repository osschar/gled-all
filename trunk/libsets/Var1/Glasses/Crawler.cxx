// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Crawler.h"
#include "Crawler.c7"

#include "Tringula.h"
#include "ParaSurf.h"

// Crawler

//______________________________________________________________________________
//
//

ClassImp(Crawler);

//==============================================================================

void Crawler::_init()
{
  mLevH = 0.1f;

  mRayOffset = 0;
}

Crawler::Crawler(const Text_t* n, const Text_t* t) :
  Dynamico(n, t)
{
  _init();
}

Crawler::~Crawler()
{}

//==============================================================================

void Crawler::SetTringula(Tringula* tring)
{
  // Set tringula to which the extendio is attached.
  // Sub-classes override this to reinitialize cached data.

  PARENT_GLASS::SetTringula(tring);

  mRayOffset = 2.0f * mTringula->GetMesh()->GetTTvor()->mMaxEdgeLen;
}

//==============================================================================

void Crawler::TimeTick(Double_t t, Double_t dt)
{
  static const Exc_t _eh("Crawler::TimeTick ");

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

  step_length   = velocity_mag * dt + mExtraStep;
  mExtraStep    = 0;

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

  if (mGrav.DecaySafeties(dt, step_length))
  {
    mTringula->GetParaSurf()->pos2grav(pos, mGrav);

    Float_t vl = mGrav.Dir() | velocity;  if (vl < 0) vl = -vl;
    Float_t vt = sqrtf(velocity_mag2 - vl*vl);

    update_grav_safeties(vl, vt);

    // Testing printout.
    if (bSelected && G_DEBUG > 0)
    {
      mGrav.Print();
      printf("  v_mag=%f, vl=%f, vt=%f, t_safe=%f, d_safe=%f (d/v)=%f\n",
             velocity_mag, vl, vt,  mGrav.fSafeTime, mGrav.fSafeDistance,
             mGrav.fSafeDistance / velocity_mag);
    }
  }

  Opcode::Ray R;
  R.mDir  = mGrav.Dir();
reoffset:
  R.mOrig.Msc(pos, R.mDir, mRayOffset);;

  TriMesh* terrain_mesh = mTringula->GetMesh();

  UInt_t cache = mOPCRCCache;
  if (RC.Collide(R, *terrain_mesh->GetOPCModel(), 0, &mOPCRCCache))
  {
    if (CF.GetNbFaces() == 1)
    {
      const CollisionFace& cf = CF.GetFaces()[0];
      pos.TMac(R.mDir, cf.mDistance - mRayOffset - mLevH);

      if (cache != mOPCRCCache)
      {
        Float_t* n = terrain_mesh->GetTTvor()->TriangleNormal(mOPCRCCache);
        mTrans.SetBaseVec(3, n);
        mTrans.OrtoNorm3Column(2, 3);
        mTrans.SetBaseVecViaCross(1);
      }
    }
    else
    {
      ISmess(_eh + RC.CollideInfo(true, R) + "\nIncreasing ray-offset.");
      mRayOffset *= 2;
      goto reoffset;
    }
  }
  else
  {
    ISwarn(_eh + RC.CollideInfo(false, R));
  }
}

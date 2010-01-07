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
  mDriveMode = DM_ConstVelocities;

  mLevH = 0.1f;

  mRayOffset = 0;

  mThrottle.SetMinMaxDelta(-1, 4, 1);
  mWheel   .SetMinMaxDelta(-1, 1, 0.4);
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

  const Float_t dtf = dt;

  // Controllers
  mThrottle.TimeTick(dtf);
  mWheel.   TimeTick(dtf);

  switch (mDriveMode)
  {
    case DM_Parked:
      mVVec.Set(0,0,0);
      break;

    case DM_ConstVelocities:
      break;

    case DM_Controllers:
      // This is uber fake ... just to get stuff connected.
      mVVec.Set(mThrottle.Get(), 0, 0);
      mWVec.Set(mWheel.Get(), 0, 0);
      break;
  }

  using namespace Opcode;

  RayCollider    RC;
  RC.SetTemporalCoherence(true);
  RC.SetClosestHit(true);
  CollisionFaces CF;
  RC.SetDestination(&CF);

  Opcode::Point velocity; // Velocity in master frame.
  Float_t       velocity_mag2, velocity_mag, step_length;

  mTrans.RotateVec3(mVVec, velocity);
  velocity_mag2 = velocity.SquareMagnitude();
  velocity_mag  = sqrtf(velocity_mag2);

  step_length   = velocity_mag * dtf + mExtraStep;
  mExtraStep    = 0;

  if (mDriveMode == DM_Controllers)
  {
    // !!!! The factor 2 below should really be inertia tensor dependant and the force
    // should be calculated and go into mWVec ... or sth.
    Float_t dp;
    dp = mTerrainUp.Dot(mTrans.PtrBaseVecX());
    if (fabsf(dp) > 0.001f)
      mTrans.RotateLF(1, 3, 2.0f*dtf*(acosf(dp) - HALFPI));
    dp = mTerrainUp.Dot(mTrans.PtrBaseVecY());
    if (fabsf(dp) > 0.001f)
      mTrans.RotateLF(2, 3, 2.0f*dtf*(acosf(dp) - HALFPI));
  }

  // Use velocity - is needed afterwards, too.
  // [But can be changed by CheckBoundaries()!]
  // mTrans.Move3LF(mVVec.x * dtf, mVVec.y * dtf, mVVec.z * dtf);
  mTrans.Move3PF(velocity.x * dtf, velocity.y * dtf, velocity.z * dtf);

  mTrans.RotateLF(1, 2, dtf*mWVec.x);
  mTrans.RotateLF(2, 3, dtf*mWVec.y);
  mTrans.RotateLF(3, 1, dtf*mWVec.z);

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

  if (mGrav.DecaySafeties(dtf, step_length))
  {
    mTringula->GetParaSurf()->pos2grav(pos, mGrav);

    Float_t vl = mGrav.Dir() | velocity;  if (vl < 0) vl = -vl;
    Float_t vt = sqrtf(velocity_mag2 - vl*vl);

    update_grav_safeties(vl, vt);

    // Testing printout.
    if (mDebugBits & DB_GravData)
    {
      mGrav.Print();
      printf("  v_mag=%f, vl=%f, vt=%f, t_safe=%f, d_safe=%f (d/v)=%f\n",
             velocity_mag, vl, vt,  mGrav.fSafeTime, mGrav.fSafeDistance,
             mGrav.fSafeDistance / velocity_mag);
    }
  }

  Opcode::Ray R;
  R.mDir  = mGrav.Dir();
  Int_t reoffset_count = 0;
reoffset:
  R.mOrig.Msc(pos, R.mDir, mRayOffset);

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
        mTerrainUp.Set(terrain_mesh->GetTTvor()->TriangleNormal(mOPCRCCache));
	if (mDriveMode != DM_Controllers)
	{
	  mTrans.SetBaseVec(3, mTerrainUp);
	  mTrans.OrtoNorm3Column(2, 3);
	  mTrans.SetBaseVecViaCross(1);
	}
      }
    }
    else
    {
      if (mTringula->GetParaSurf()->IsValidPos(pos))
      {
	if (reoffset_count < 5)
	{
	  ISmess(_eh + RC.CollideInfo(true, R) + GForm(" Increasing ray-offset from %f for '%s'.", mRayOffset, GetName()));
	  mRayOffset *= 2;
	  ++reoffset_count;
	  goto reoffset;
	}
	else
	{
	  // This probably means Crawler has fallen off but opcode
	  // and parasurf perceive this somewhat differently.
	  // This happens in tringula test case (tringula.C(99)).
	  // Could move the edge planes slightly more inwards.
	  // But then again, the idea is that triangular patches bind into
	  // hierarchy of tringulas where such case would mean we are entering
	  // a new patch.
	  ISmess(_eh + RC.CollideInfo(true, R) + GForm(" Increasing ray-offset did not help - parking '%s'", GetName()));
	  SetDriveMode(DM_Parked);
	}
      }
      else
      {
	ISmess(_eh + RC.CollideInfo(true, R) + GForm(" Fallen off - parking, ray-offset was %f for '%s'.", mRayOffset, GetName()));
	SetDriveMode(DM_Parked);
      }
    }

    if (reoffset_count != 0)
    {
      mRayOffset = 2.0f * mTringula->GetMesh()->GetTTvor()->mMaxEdgeLen;
    }
  }
  else
  {
    ISwarn(_eh + RC.CollideInfo(false, R));
  }
}

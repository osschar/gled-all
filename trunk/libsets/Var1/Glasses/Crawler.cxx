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
  mLevH   = 0.1f;
}

Crawler::Crawler(const Text_t* n, const Text_t* t) :
  Dynamico(n, t)
{
  _init();
}

Crawler::~Crawler()
{}

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

  Float_t path = dt * mV;

  mTrans.MoveLF(1, path);
  mTrans.RotateLF(1, 2, dt*mW);

  mSafety -= path;
  if (mSafety < 0)
  {
    // Check boundaries - this can result in tringula switch.
    Bool_t trans_changed = mTringula->CheckBoundaries(this, mSafety);

    if (trans_changed)
    {
      // Invalidate position dependant caches.
      // GravData should be fine, but could force recalc, too.
    }
  }

  Opcode::Point& pos = * (Opcode::Point*) ref_trans().ArrT();

  Opcode::Ray R;
  Float_t ray_offset = mTringula->GetParaSurf()->pos2hray(pos, R);
  // !!! This is potentially expensive, e.g., for torus.
  // Should cache down direction while in the same triangle.

  TriMesh* terrain_mesh = mTringula->GetMesh();

  UInt_t cache = mOPCRCCache;
  if (RC.Collide(R, *terrain_mesh->GetOPCModel(), 0, &mOPCRCCache))
  {
    if (CF.GetNbFaces() == 1)
    {
      const CollisionFace& cf = CF.GetFaces()[0];
      pos.TMac(R.mDir, cf.mDistance - ray_offset - mLevH);

      if (cache != mOPCRCCache)
      {
        Float_t* n = terrain_mesh->GetTTvor()->TriangleNormal(mOPCRCCache);
        mTrans.SetBaseVec(3, n);
        mTrans.OrtoNorm3Column(2, 3);
        mTrans.SetBaseVecViaCross(1);
      }
    }
  }
  else
  {
    printf("%scollide status=<failed>, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
           _eh.Data(),
           RC.GetContactStatus(),
           RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(),
           RC.GetNbIntersections());
  }
}

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Dynamico
//
//

#include "Dynamico.h"
#include "Dynamico.c7"

#include <Stones/TringTvor.h>

#include <TMath.h>

ClassImp(Dynamico);

/**************************************************************************/

void Dynamico::_init()
{
  mLastTransPtr = &mLastTrans;
  mLastAABBPtr  = &mLastAABB;

  bParked = false;

  mV = mW = 0.0f;
  mLevH   = 0.1f;

  mMoveMode = MM_Crawl;

  mOPCRCCache = OPC_INVALID_ID;
}

/**************************************************************************/

bool Dynamico::handle_collision(Opcode::RayCollider    & ray_collider,
                                Opcode::Ray            & ray,
                                Opcode::Point          & com,
                                CollisionSegments      & segments,
                                Extendio               * extendio)
{
  // Handle collision with a static object extendio.
  //
  // If com is inside extendio, let it go, we're too late (or it
  // was inside to start with).
  // If collision-vertex is in front of com bounce off with change of
  // direction, otherwise just move away from collision-vertex.
  //
  // Input:
  //   com      center-of-mass of this dynamico
  // Return:    false if ray collision fails.
  //
  // !!!!! For the moment this is also used for dyno-dyno collision.
  // When fixed, this should take Statico* and the new method Dyno*.

  static const Exc_t _eh("Dynamico::handle_collision ");

  if (ray_collider.Collide(ray, *extendio->get_opc_model(), extendio->RefTrans()))
  {
    using namespace Opcode;

    Opcode::CollisionFaces& CF = * ray_collider.GetDestination();

    if (bVerboseCollide)
      printf("%sCollide shooting ray outwards at the hit mesh from COM, ni=%d.\n",
             _eh.Data(), CF.GetNbFaces());

    // Do fixing only when nfaces is even, i.e., dyno's com is outside.
    if (CF.GetNbFaces() % 2 == 0)
    {
      // center-vertex of collision segments.
      const Point& cv = segments.RefCenter();

      Point com2cv(cv); com2cv -= com;

      Point& fwd = * (Point*) mTrans.PtrBaseVec(1);

      if ((fwd | com2cv) * mV > 0 && segments.size() > 1)
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
        if ((cross | *(Point*)mTrans.PtrBaseVec(3)) < 0)
          mTrans.RotateLF(2, 1, TMath::PiOver2() - TMath::ASin(cp));
        else
          mTrans.RotateLF(1, 2, TMath::PiOver2() - TMath::ASin(cp));
      }
      else
      {
        // CV is behind - move away from it.
        // !!! 0.1 should be replaced with a ray from cv back towards com.
        com2cv *= -0.1f;
        mTrans.Move3PF(com2cv.x, com2cv.y, com2cv.z);
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}

/**************************************************************************/

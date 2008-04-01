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

#include "Statico.h"

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

    if (dyno->bVerboseCollide)
      printf("%sCollide shooting ray outwards at the hit mesh from COM, ni=%d.\n",
             _eh.Data(), CF.GetNbFaces());

    // Do fixing only when nfaces is even, i.e., dyno's com_dyno is outside.
    if (CF.GetNbFaces() % 2 == 0)
    {
      // center-vertex of collision segments.
      const Point& cv = segments.RefCenter();

      Point com2cv(cv); com2cv -= com_dyno;

      HTransF& t_dyno = dyno->mTrans;

      Point& fwd = * (Point*) t_dyno.PtrBaseVec(1);

      if ((fwd | com2cv) * dyno->mV > 0 && segments.size() > 1)
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
      }
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
  Point v0(t0.PtrBaseVec(1)); v0 *= dyno0->mV; 
  Point v1(t1.PtrBaseVec(1)); v1 *= dyno1->mV;
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

    v0.TMac(dv_dir, w0p);
    Float_t vm0 = v0.Magnitude();
    v0 *= 1.0f / vm0;
    // if (dyno0->mV < 0) { v0.Neg(), vm0 = -vm0; }
    t0.SetBaseVec(1, v0);
    t0.OrtoNorm3Column(1, 3);
    t0.SetBaseVecViaCross(2);
    dyno0->mV = vm0;

    v1.TMac(dv_dir, w1p - w1);
    Float_t vm1 = v1.Magnitude();
    v1 *= 1.0f / vm1;
    // if (dyno1->mV < 0) { v1.Neg(), vm1 = -vm1; }
    t1.SetBaseVec(1, v1);
    t1.OrtoNorm3Column(1, 3);
    t1.SetBaseVecViaCross(2);
    dyno1->mV = vm1;
  } else {
    // they are moving apart, just separate them some more
    dcomp *= 0.1f;
    t0.Move3PF(-dcomp.x, -dcomp.y, -dcomp.z);
    t1.Move3PF( dcomp.x,  dcomp.y,  dcomp.z);
  }

  return true;
}

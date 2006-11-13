// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Tringula
//
// Many functions implicitly assume rectangular ares limited by
// 4 edge-planes defined in SetEdgePlanes(RectTerrain*).

#include "Tringula.h"
#include <Glasses/ZHashList.h>
#include <Glasses/RectTerrain.h>
#include <Stones/TringTvor.h>
#include "TriMesh.h"
#include "Dynamico.h"
#include "Tringula.c7"

#include <Glasses/ZQueen.h>

#include <Opcode/Opcode.h>


ClassImp(Tringula);

/**************************************************************************/

void Tringula::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;
  
  bPreferSmooth = false;

  mOPCCFaces = 0;
  mOPCRayCol = 0;

  bRnrRay = false;
  mRayL = 20; mRayT = -90; mRayP = 0;

  bRnrDynos = bPickDynos = true;

  mNumEPlanes = 0;
  mEdgePlanes = 0;
  mEdgeRule   = ER_Stop;

  mRndGen.SetSeed(0);
}

Tringula::~Tringula()
{
  delete mOPCCFaces;
  delete mOPCRayCol;
  delete [] mEdgePlanes;
}

void Tringula::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mDynos == 0) {
    ZHashList* l = new ZHashList("Dynos", GForm("Dynos of Tringula %s", GetName()));
    l->SetElementFID(Dynamico::FID());
    mQueen->CheckIn(l);
    SetDynos(l);
  }
}

/**************************************************************************/

void Tringula::get_ray_dir(Float_t* d, Float_t len)
{
  if(len == 0) len = mRayL;
  using namespace TMath;
  Double_t th = DegToRad()*mRayT;
  Double_t ph = DegToRad()*mRayP;
  Float_t  ct = Cos(th);
  d[0] = len * ct * Cos(ph);
  d[1] = len * ct * Sin(ph);
  d[2] = len * Sin(th);
}

/**************************************************************************/

void Tringula::MakeOpcodeModel()
{
  static const Exc_t _eh("Tringula::MakeOpcodeModel ");

  if (mMesh == 0)
    throw(_eh + "mMesh is null.");

  using namespace Opcode;

  mMesh->BuildOpcStructs();

  mOPCRayCol = new RayCollider;
  {
    RayCollider& RC = *mOPCRayCol;
    RC.SetFirstContact(true);
    RC.SetTemporalCoherence(true);

    if(mOPCCFaces == 0)
      mOPCCFaces = new CollisionFaces;
    RC.SetDestination(mOPCCFaces);
  }
}

void Tringula::RayCollide()
{
  static const Exc_t _eh("Tringula::RayCollide ");

  if(mMesh == 0 || mMesh->GetOPCModel() == 0)
    throw(_eh + "Opcode model not created.");

  using namespace Opcode;

  RayCollider RC;
  RC.SetFirstContact(false);  // true to only take first hit (not closest!)
  RC.SetClosestHit(true);     // to sort the hits by distance
  RC.SetCulling(false);

  // primitive tests on by default

  if(mOPCCFaces == 0)
    mOPCCFaces = new CollisionFaces;
  RC.SetDestination(mOPCCFaces);

  Opcode::Ray R;
  mRayPos.GetXYZ((Float_t*)&R.mOrig.x);
  get_ray_dir((Float_t*)&R.mDir, 1);

  const char* setval = RC.ValidateSettings();
  printf("RayCollider::ValidateSettings: %s\n", setval ? setval : "OK.");

  bool status;
  status = RC.Collide(R, *mMesh->GetOPCModel());
           // default-args: const Matrix4x4* world=null, udword* cache=null);

  printf("collide status=%d, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
         status, RC.GetContactStatus(),
         RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections());

  CollisionFaces& CF = *mOPCCFaces;
  printf("n faces = %d\n", CF.GetNbFaces());
  for(UInt_t f=0; f<CF.GetNbFaces(); ++f) {
    const CollisionFace& cf = CF.GetFaces()[f];
    printf("  %2d %6d  %10f  %10f %10f\n",
           f, cf.mFaceID, cf.mDistance, cf.mU, cf.mV);
  }
  Stamp(FID());
}

/**************************************************************************/

Dynamico* Tringula::RandomDynamico(Float_t v_min, Float_t v_max, Float_t w_max)
{
  Double_t x = mRndGen.Uniform(mEdgePlanes[1].d, -mEdgePlanes[3].d);
  Double_t y = mRndGen.Uniform(mEdgePlanes[0].d, -mEdgePlanes[2].d);

  Double_t phi = mRndGen.Uniform(0, TMath::TwoPi());
  Double_t cos = TMath::Cos(phi), sin = TMath::Sin(phi);

  Dynamico* d = new Dynamico(GForm("Dynamico %d", mDynos->GetSize() + 1));
  ZTrans& t = d->ref_trans();
  t.SetBaseVec(1, cos,  sin, 0);
  t.SetBaseVec(2, 0,      0, 1);
  t.SetBaseVec(1, sin, -cos, 0);
  t.SetBaseVec(4, x, y, 0);
  d->SetV(mRndGen.Uniform( v_min, v_max));
  d->SetW(mRndGen.Uniform(-w_max, w_max));

  place_on_terrain(*d);

  mQueen->CheckIn(d);
  d->SetMesh(*mDefDynMesh);
  mDynos->Add(d);

  return d;
}

/**************************************************************************/

void Tringula::SetEdgePlanes(RectTerrain* rect_terr)
{
  // Set bounding-planes for x/y coordinates.
  //
  // Assumes RectTerrain *always* stretches from 0,0 into the positive
  // direction (which is so far true, at least for TrinTvor, rendering
  // can be offset). Oh yess ... be carefull about the border,
  // too, as its role is not very well defined right now.

  static const Exc_t _eh("Tringula::SetEdgePlanes ");

  if (rect_terr == 0)
    throw(_eh + "null argument");

  if (mEdgePlanes) delete [] mEdgePlanes;
  mNumEPlanes = 4;
  mEdgePlanes = new Opcode::Plane[4];
  RectTerrain& R = *rect_terr;
  mEdgePlanes[0].Set( 0, -1, 0, 0);
  mEdgePlanes[1].Set(-1,  0, 0, 0);
  mEdgePlanes[2].Set( 0,  1, 0, - (R.GetNy() - 1) * R.GetDy());
  mEdgePlanes[3].Set( 1,  0, 0, - (R.GetNx() - 1) * R.GetDx());
}

/**************************************************************************/

void Tringula::TimeTick(Double_t t, Double_t dt)
{
  GLensWriteHolder wlck(this);

  Stepper<Dynamico> stepper(*mDynos);
  while (stepper.step()) {

    using namespace Opcode;

    Dynamico& D = **stepper;

    Point old_pos, pos;

    D.mTrans.GetPos(old_pos);

    if (D.mV != 0) {
      D.mTrans.MoveLF(1, dt * D.mV);
    }
    if (D.mW != 0) {
      D.mTrans.RotateLF(3, 1, dt*D.mW);
    }

    D.mTrans.GetPos(pos);

    Point vertical(0, 0, -1);

    // printf("  Distances: ");
    for (Int_t p=0; p<mNumEPlanes; ++p) {
      Float_t d = mEdgePlanes[p].Distance(pos);
      // printf("%6.2f ", d);
      if (d > 0) handle_edge_crossing(D, old_pos, pos, p, d);
    }
    // printf("\n");

    RayCollider&    RC = *mOPCRayCol;
    CollisionFaces& CF = *mOPCCFaces;

    static const Float_t ray_offset = 100;

    Opcode::Ray R(pos, vertical);
    R.mOrig.z += ray_offset;

    UInt_t cache = D.mOPCRCCache;
    if ( RC.Collide(R, *mMesh->GetOPCModel(), 0, &D.mOPCRCCache) ) {

      if (CF.GetNbFaces() == 1) {
        const CollisionFace& cf = CF.GetFaces()[0];
        pos.z += D.mLevH - cf.mDistance + ray_offset;

        if (cache != D.mOPCRCCache) {
          Float_t* n = mMesh->GetTTvor()->TriangleNormal(D.mOPCRCCache);
          D.mTrans.SetBaseVec(2, n);
          D.mTrans.Orto3Column(1, 2);
          D.mTrans.Norm3Column(1);
          D.mTrans.SetBaseVecViaCross(3);
        }
      }

    } else {

      printf("collide status=<failed>, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
             RC.GetContactStatus(),
             RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections());

    }

    D.mTrans.SetPos(pos);

  } // end while dynos

}

void Tringula::handle_edge_crossing
( Dynamico& D, Opcode::Point& old_pos, Opcode::Point& pos, 
  Int_t plane, Float_t dist )
{
  Opcode::Plane& P = mEdgePlanes[plane];

  switch (mEdgeRule)
    {
    case ER_Stop: {
      pos -= dist*P.n;
      D.mV = 0;
      D.mW = 0;
      D.Stamp(D.FID());
      break;
    }
    case ER_Hold: {
      Opcode::Point step = pos; step -= old_pos;
      Float_t depth = fabsf(P.Distance(old_pos));
      pos = old_pos + depth*step/(depth+dist);
      break;
    }
    case ER_Bounce: {
      pos -= 2*dist*P.n;
      // Rotate accordingly
      ZTrans& t = D.ref_trans();
      Opcode::Point fwd;
      t.GetBaseVec(1, (Float_t*) fwd);
      fwd += -2 * (fwd | P.n) * P.n;
      t.SetBaseVec(1, (Float_t*) fwd);
      t.Orto3Column(2, 1);
      t.Norm3Column(2);
      t.SetBaseVecViaCross(3);
      break;
    }
    case ER_Wrap: {
      // Cross-over the opposite plane, assume 4 planes pre-defined
      // rectangular layout.
      Int_t opp_plane = (plane + 2) % mNumEPlanes;
      Opcode::Plane& O = mEdgePlanes[opp_plane];
      Float_t opp_dist = O.Distance(pos);
      pos += dist * P.n - opp_dist * O.n;
      break;
    }

    } // end switch
}

/**************************************************************************/

void Tringula::place_on_terrain(Dynamico& D)
{
  using namespace Opcode;

  Point pos;
  Point vertical(0, 0, -1);

  D.mTrans.GetPos(pos);
  RayCollider&    RC = *mOPCRayCol;
  CollisionFaces& CF = *mOPCCFaces;

  static const Float_t ray_offset = 100;

  Opcode::Ray R(pos, vertical);
  R.mOrig.z += ray_offset;

  UInt_t cache = D.mOPCRCCache;
  if ( RC.Collide(R, *mMesh->GetOPCModel(), 0, &D.mOPCRCCache) ) {

    if (CF.GetNbFaces() == 1) {
      const CollisionFace& cf = CF.GetFaces()[0];
      pos.z += D.mLevH - cf.mDistance + ray_offset;

      if (cache != D.mOPCRCCache) {
        Float_t* n = mMesh->GetTTvor()->TriangleNormal(D.mOPCRCCache);
        D.mTrans.SetBaseVec(2, n);
        D.mTrans.Orto3Column(1, 2);
        D.mTrans.Norm3Column(1);
        D.mTrans.SetBaseVecViaCross(3);
      }
    }

  } else {

    printf("collide status=<failed>, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
           RC.GetContactStatus(),
           RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections());

  }
}

/**************************************************************************/


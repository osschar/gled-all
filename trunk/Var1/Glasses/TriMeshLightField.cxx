// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TriMeshLightField
//
//

#include "TriMeshLightField.h"
#include <Glasses/ZNode.h>
#include "TriMeshLightField.c7"
#include "TriMesh.h"

#include <Stones/TringTvor.h>

#include <Opcode/Opcode.h>

ClassImp(TriMeshLightField)

/**************************************************************************/

void TriMeshLightField::_init()
{
  mAmbiLit      = 0.15;
  mAmbiShadowed = 0.1;
  mAmbiBackFace = 0.05;

  mLampPos.SetXYZ(0, 0, 0);
  bDirectional = false;
}

/**************************************************************************/

void TriMeshLightField::ModulateTvor(Bool_t regen_tring_cols)
{
  static const Exc_t _eh("TriMeshLightField::ModulateTvor ");

  if (mDim != 1) throw(_eh + "unsupported dimension.");

  assert_mesh(_eh);
  TringTvor& TT = * mMesh->GetTTvor();

  Float_t* F = FVec();
  UChar_t* C = TT.mCols;
  for (Int_t i=0; i<TT.mNVerts; ++i, ++F, C+=4)
  {
    const Float_t f = * F;
    C[0] = UChar_t(f*C[0]);  C[1] = UChar_t(f*C[1]);  C[2] = UChar_t(f*C[2]);
  }

  if (regen_tring_cols && TT.HasTringCols())
    TT.GenerateTriangleColorsFromVertexColors();

  mMesh->StampReqTring(TriMesh::FID());
}

void TriMeshLightField::PartiallyModulateTvor(set<Int_t> vertices,
                                              Bool_t regen_tring_cols)
{
  static const Exc_t _eh("TriMeshLightField::PartiallyModulateTvor ");

  if (mDim != 1) throw(_eh + "unsupported dimension.");

  assert_mesh(_eh);
  TringTvor& TT = * mMesh->GetTTvor();

  for (set<Int_t>::iterator i=vertices.begin(); i!=vertices.end(); ++i)
  {
    const Float_t f = * FVec(*i);
    UChar_t* C = TT.Color(*i);
    C[0] = UChar_t(f*C[0]);  C[1] = UChar_t(f*C[1]);  C[2] = UChar_t(f*C[2]);
  }

  // !!!! Very sub-optimal. Need to introduce edge-data with
  // !!!! triangle idcs, optimised for two.
  if (regen_tring_cols && TT.HasTringCols())
    TT.GenerateTriangleColorsFromVertexColors();

  mMesh->StampReqTring(TriMesh::FID());
}

/**************************************************************************/

void TriMeshLightField::CalculateLightField()
{
  static const Exc_t _eh("TriMeshLightField::CalculateLightField ");

  if (mDim != 1) throw(_eh + "unsupported dimension.");

  assert_mesh(_eh);
  TringTvor& TT = * mMesh->GetTTvor();

  Opcode::RayCollider RC;
  RC.SetCulling   (false);
  RC.SetClosestHit(true);

  Opcode::CollisionFaces CF;
  RC.SetDestination(&CF);

  Opcode::Ray   R;
  Opcode::Point nrdir(0, 0, 0); // normalized ray-direction

  if (bDirectional)
  {
    TT.AssertBoundingBox();
    Float_t max_dist = TT.BoundingBoxDiagonal();

    mLampPos.GetXYZ((Float_t*)&R.mDir.x);
    nrdir   = R.mDir.Normalize();
    R.mDir *= - max_dist;
  }
  else
  {
    mLampPos.GetXYZ((Float_t*)&R.mOrig.x);
  }

  Float_t* F = FVec();
  Float_t* V = TT.mVerts;
  Float_t* N = TT.mNorms;
  for (Int_t i=0; i<TT.mNVerts; ++i, ++F, V+=3, N+=3)
  {
    using namespace Opcode;
    Point& v = * (Opcode::Point*) V;
    Point& n = * (Opcode::Point*) N;

    if (bDirectional)
    {
      R.mOrig.Msc(v, R.mDir, 0.9999f);
    }
    else
    {
      R.mDir.Sub(v, R.mOrig);
      Float_t dist = R.mDir.Magnitude();
      nrdir   =  R.mDir;
      nrdir  *= -1.0f/dist;
      R.mDir *=  1.001f;
    }

    Float_t cos_alpha = n.Dot(nrdir);

    if (cos_alpha > 0)
    { // front-facing
      if ( ! RC.Collide(R, *mMesh->GetOPCModel()) )
        throw(_eh + "collider failed.");
      if (CF.GetNbFaces() == 0 || CF.GetFaces()[0].mDistance > 0.999f)
        *F = TMath::Max(cos_alpha, mAmbiLit);
      else
        *F = mAmbiShadowed;
    }
    else
    { // back-facing
      *F = mAmbiBackFace;
    }
  }
}

/**************************************************************************/

void TriMeshLightField::SetupLampPos(ZNode* lamp, ZNode* mesh)
{
  // Setup mLampPos for positional light at node 'lamp' and mesh at
  // node 'mesh'.
  // Member 'bDirectional' is not changed!

  static const Exc_t _eh("TriMeshLightField::SetupLampPos ");

  if (lamp == 0 || mesh == 0)
    throw(_eh + "requires non-null arguments.");

  auto_ptr<ZTrans> t (ZNode::BtoA(mesh, lamp));
  if (t.get() == 0)
    throw(_eh + "no path between arguments.");

  t->GetPos(mLampPos);
  Stamp(FID());
}

void TriMeshLightField::SetupLampDir(ZNode* mesh, Float_t theta, Float_t phi)
{
  // Setup mLampPos for directional light and mesh at node 'mesh'.
  // Arguments 'theta' and 'phi' determine direction towards the light
  // source in parent frame of the 'mesh'.
  // Member 'bDirectional' is not changed!

  static const Exc_t _eh("TriMeshLightField::SetupLampPos ");

  if (mesh == 0)
    throw(_eh + "requires non-null mesh argument.");

  ZTrans t(mesh->RefTrans());
  t.Invert();

  Double_t ct = TMath::Cos(theta);
  mLampPos.SetXYZ(ct*TMath::Cos(phi), ct*TMath::Sin(phi), TMath::Sin(theta));
  t.MultiplyIP(mLampPos);
  Stamp(FID());
}

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TriMeshField
//
//

#include "TriMeshField.h"
#include <Glasses/TriMesh.h>
#include <Glasses/RGBAPalette.h>
#include "TriMeshField.c7"

#include <TRandom.h>
#include <TF3.h>

ClassImp(TriMeshField)

/**************************************************************************/

void TriMeshField::_init()
{
  mNVerts = 0; mDim = 1;

  mMinValue = 0;
  mMaxValue = 1;
}

void TriMeshField::Resize(Int_t nvert, Int_t dim)
{
  if (mNVerts != nvert || mDim != dim)
  {
    mNVerts = nvert;
    mDim    = dim;
    mField.resize(mDim*mNVerts);
    Stamp(FID_t());
  }
}

void TriMeshField::ResizeToMesh(Int_t dim)
{
  // Resize field to size of mMesh and dim.
  // If dim <= 0, keep the old dim;

  static const Exc_t _eh("TriMeshField::ResizeToMesh ");

  assert_mesh(_eh);
  if (mMesh->GetTTvor() == 0)
    throw(_eh + "TringTvor does not exist.");

  if (dim <= 0) dim = mDim;

  Resize(mMesh->GetTTvor()->mNVerts, dim);
}


/**************************************************************************/

void TriMeshField::SetField(Float_t c0)
{
  static const Exc_t _eh("TriMeshField::SetField ");

  if (mDim != 1)
    throw(_eh + "argument count incompatible with field dimension");

  Float_t* F = FVec();
  for (Int_t i=0; i<mNVerts; ++i, ++F)
    F[0] = c0;
}

void TriMeshField::SetField(Float_t c0, Float_t c1)
{
  static const Exc_t _eh("TriMeshField::SetField ");

  if (mDim != 2)
    throw(_eh + "argument count incompatible with field dimension");

  Float_t* F = FVec();
  for (Int_t i=0; i<mNVerts; ++i, F+=mDim)
  {
    F[0] = c0; F[1] = c1;
  }
}

void TriMeshField::SetField(Float_t c0, Float_t c1, Float_t c2)
{
  static const Exc_t _eh("TriMeshField::SetField ");

  if (mDim != 3)
    throw(_eh + "argument count incompatible with field dimension");

  Float_t* F = FVec();
  for (Int_t i=0; i<mNVerts; ++i, F+=mDim)
  {
    F[0] = c0; F[1] = c1; F[2] = c2;
  }
}

/**************************************************************************/

namespace
{
inline Float_t std_val_1d(Float_t* F) { return F[0]; }
inline Float_t std_val_2d(Float_t* F) { return sqrtf(F[0]*F[0] + F[1]*F[1]); }
inline Float_t std_val_3d(Float_t* F) { return sqrtf(F[0]*F[0] + F[1]*F[1] + F[2]*F[2]); }
}

void TriMeshField::FindMinMaxField()
{
  static const Exc_t _eh("TriMeshField::FindMinMaxField ");

  if (mNVerts <= 0)
  {
    mMinValue = mMaxValue = 0;
  }
  else
  {
    Float_t* F = FVec();

    if (mFormula.IsNull())
    {
      switch (mDim)
      {
        case 1:
          mMinValue = mMaxValue = std_val_1d(F);
          ++F;
          for (Int_t i=1; i<mNVerts; ++i, ++F)
            check_min_max(std_val_1d(F));
          break;
        case 2:
          mMinValue = mMaxValue = std_val_2d(F);
          F += mDim;
          for (Int_t i=1; i<mNVerts; ++i, F+=mDim)
            check_min_max(std_val_2d(F));
          break;
        case 3:
          mMinValue = mMaxValue = std_val_3d(F);
          F += mDim;
          for (Int_t i=1; i<mNVerts; ++i, F+=mDim)
            check_min_max(std_val_3d(F));
          break;
        default:
          throw(_eh + "unsupported dimension.");
          break;
      }
    }
    else
    {
      TF3 tf3(GForm("TriMeshField_CT_%d", GetSaturnID()), mFormula, 0, 0);

      switch (mDim)
      {
        case 1:
          mMinValue = mMaxValue = (Float_t) tf3.Eval(F[0]);
          ++F;
          for (Int_t i=1; i<mNVerts; ++i, ++F)
            check_min_max((Float_t) tf3.Eval(F[0]));
          break;
        case 2:
          mMinValue = mMaxValue = (Float_t) tf3.Eval(F[0], F[1]);
          F += mDim;        
          for (Int_t i=1; i<mNVerts; ++i, F+=mDim)
            check_min_max((Float_t) tf3.Eval(F[0], F[1]));
          break;
        case 3:
          mMinValue = mMaxValue = (Float_t) tf3.Eval(F[0], F[1], F[2]);
          F += mDim;
          for (Int_t i=1; i<mNVerts; ++i, F+=mDim)
            check_min_max((Float_t) tf3.Eval(F[0], F[1], F[2]));
          break;
        default:
          throw(_eh + "unsupported dimension.");
          break;
      }
    }
  }

  Stamp(FID());
}

/**************************************************************************/

void TriMeshField::ColorizeTvor(Bool_t regen_tring_cols)
{
  static const Exc_t _eh("TriMeshField::ColorizeTvor ");

  assert_palette(_eh);
  assert_mesh(_eh);

  TringTvor& TT = * mMesh->GetTTvor();
  TT.AssertCols();

  mPalette->SetMinFlt(mMinValue);
  mPalette->SetMaxFlt(mMaxValue);

  Float_t* F = FVec();
  UChar_t* C = TT.mCols;

  if (mFormula.IsNull())
  {
    switch (mDim)
    {
      case 1:
        for (Int_t i=0; i<TT.mNVerts; ++i, ++F, C+=4)
          mPalette->ColorFromValue(std_val_1d(F), C);
        break;
      case 2:
        for (Int_t i=0; i<TT.mNVerts; ++i, F+=mDim, C+=4)
          mPalette->ColorFromValue(std_val_2d(F), C);
        break;
      case 3:
        for (Int_t i=0; i<TT.mNVerts; ++i, F+=mDim, C+=4)
          mPalette->ColorFromValue(std_val_3d(F), C);
        break;
      default:
        throw(_eh + "unsupported dimension.");
        break;
    }
  }
  else
  {
    TF3 tf3(GForm("TriMeshField_CT_%d", GetSaturnID()), mFormula, 0, 0);

    switch (mDim)
    {
      case 1:
        for (Int_t i=0; i<TT.mNVerts; ++i, ++F, C+=4)
          mPalette->ColorFromValue((Float_t) tf3.Eval(F[0]), C);
        break;
      case 2:
        for (Int_t i=0; i<TT.mNVerts; ++i, F+=mDim, C+=4)
          mPalette->ColorFromValue((Float_t) tf3.Eval(F[0], F[1]), C);
        break;
      case 3:
        for (Int_t i=0; i<TT.mNVerts; ++i, F+=mDim, C+=4)
          mPalette->ColorFromValue((Float_t) tf3.Eval(F[0], F[1], F[2]), C);
        break;
      default:
        throw(_eh + "unsupported dimension.");
        break;
    }
  }

  if (regen_tring_cols && TT.HasTringCols())
    TT.GenerateTriangleColorsFromVertexColors();

  mMesh->StampReqTring(TriMesh::FID());
}

void TriMeshField::PartiallyColorizeTvor(set<Int_t> vertices,
                                         Bool_t regen_tring_cols)
{
  static const Exc_t _eh("TriMeshField::PartiallyColorizeTvor ");

  assert_palette(_eh);
  assert_mesh(_eh);

  TringTvor& TT = * mMesh->GetTTvor();
  TT.AssertCols();

  mPalette->SetMinFlt(mMinValue);
  mPalette->SetMaxFlt(mMaxValue);

  UChar_t* C = TT.mCols;

  set<Int_t>::iterator i   = vertices.begin();
  set<Int_t>::iterator end = vertices.end();

  if (mFormula.IsNull())
  {
    switch (mDim)
    {
      case 1:
        for ( ; i != end; ++i)
          mPalette->ColorFromValue(std_val_1d(FVec(*i)), C+*i*4);
        break;
      case 2:
        for ( ; i != end; ++i)
          mPalette->ColorFromValue(std_val_2d(FVec(*i)), C+*i*4);
        break;
      case 3:
        for ( ; i != end; ++i)
          mPalette->ColorFromValue(std_val_3d(FVec(*i)), C+*i*4);
        break;
      default:
        throw(_eh + "unsupported dimension.");
        break;
    }
  }
  else
  {
    TF3 tf3(GForm("TriMeshField_CT_%d", GetSaturnID()), mFormula, 0, 0);

    switch (mDim)
    {
      case 1:
        for ( ; i != end; ++i) {
          Float_t* F = FVec(*i);
          mPalette->ColorFromValue((Float_t) tf3.Eval(F[0]), C+*i*4);
        }
        break;
      case 2:
        for ( ; i != end; ++i) {
          Float_t* F = FVec(*i);
          mPalette->ColorFromValue((Float_t) tf3.Eval(F[0], F[1]), C+*i*4);
        }
        break;
      case 3:
        for ( ; i != end; ++i) {
          Float_t* F = FVec(*i);
          mPalette->ColorFromValue((Float_t) tf3.Eval(F[0], F[1], F[2]), C+*i*4);
        }
        break;
      default:
        throw(_eh + "unsupported dimension.");
        break;
    }
  }

  // !!!! Very sub-optimal. Need to introduce edge-data with
  // !!!! triangle idcs, optimised for two.
  if (regen_tring_cols && TT.HasTringCols())
    TT.GenerateTriangleColorsFromVertexColors();

  mMesh->StampReqTring(TriMesh::FID());
}

/**************************************************************************/

namespace { struct GaussBlob { Float_t x, y, A, sgm, efc, nfc; }; }


void TriMeshField::FillByXYGaussBlobs(Int_t n_blobs,
                                      Float_t     A_min, Float_t     A_max,
                                      Float_t sigma_min, Float_t sigma_max,
                                      Bool_t minmax_p,   Bool_t  recolor_p)
{
  static const Exc_t _eh("TriMeshField::FillByXYGaussBlobs ");

  ResizeToMesh(1);
  SetField(0);

  TringTvor& TT = * mMesh->GetTTvor();

  TT.AssertBoundingBox();
  Float_t* bbox = TT.mMinMaxBox;

  vector<GaussBlob> blobs(n_blobs);
  {
    TRandom  r(0);
    for (Int_t i=0; i<n_blobs; ++i)
    {
      GaussBlob& B = blobs[i];
      B.x   = r.Uniform(bbox[0], bbox[3]);
      B.y   = r.Uniform(bbox[1], bbox[4]);
      B.A   = r.Uniform(A_min, A_max);
      B.sgm = r.Uniform(sigma_min, sigma_max);
      B.efc = -0.5f/(B.sgm*B.sgm);       // exp-factor
      B.nfc =  B.A /(2.50662827f*B.sgm); // norm-factor, sqrt(2pi)=2.50662827463100024
      // printf("Blob %3d: x,y=(%f,%f); A,sgm=(%f,%f); efc,nfc=(%f,%f)\n",
      //        i, B.x, B.y, B.A, B.sgm, B.efc, B.nfc);
    }
  }

  Float_t *V = TT.mVerts;
  Float_t *F = FVec();
  for (Int_t v=0; v<mNVerts; ++v, V+=3, ++F)
  {
    for (Int_t i=0; i<n_blobs; ++i)
    {
      GaussBlob& B = blobs[i];
      Float_t dx = V[0] - B.x;
      Float_t dy = V[1] - B.y;
      F[0] += B.nfc * expf((dx*dx + dy*dy)*B.efc);
    }
  }

  if (minmax_p)  FindMinMaxField();
  if (recolor_p) ColorizeTvor();
}

/**************************************************************************/

void TriMeshField::Diffuse(Float_t diff_const, Float_t dt, Bool_t limit_df)
{
  static const Exc_t _eh("TriMeshField::Diffuse ");

  if (mDim != 1)
    throw(_eh + "unsupported dimension.");

  assert_mesh(_eh);
  mMesh->AssertVertexConnections();

  printf("%sD=%f, dt=%f\n", _eh.Data(), diff_const, dt);

  vector<Float_t> delta(mField.size());
  const vector<TriMesh::VertexData>& vertex_data_vec = mMesh->RefVDataVec();

  Float_t Ddt = diff_const*dt;

  for (Int_t i=0; i<mNVerts; ++i)
  {

    const TriMesh::VertexData&                 vdata = vertex_data_vec[i];
    vector<TriMesh::VConnData>::const_iterator   vcd = vdata.fVConns.begin();
    while (vcd != vdata.fVConns.end())
    {
      Int_t   j       = vcd->fVTarget;

      if (limit_df)
      {
        // Diffuse, but keep minimum value at zero.
        // This is reasonable for non-negative fields (eg. gas concentration).

        // Each edge is visited twice; the action is taken when
        // "giving field away", as limits can be applied there more
        // sensibly.

        // Ddt can be negative!
        Float_t delta_f = (mField[j] - mField[i])*Ddt;
        if (delta_f < 0)
        {
          Float_t df     = delta_f*vcd->fSpread/vcd->fDistance;
          Float_t lim_df = TMath::Max(-mField[i]*vcd->fSurface/vdata.fSurface,
                                      0.5f*delta_f);
          if (df < lim_df)
            df = lim_df;
          delta[i] += df;
          delta[j] -= df;
        }
      }
      else
      {
        // Diffuse without limits; explodes for large time steps.

        // Visit each edge only once. Should introduce edge object / array.
        if (j > i)
        {
          // For now we forget about the "capacity" (surface) belonging
          // to each vertex, but we take into account the spread of the
          // edge and its length.
          // These should all be optional.
          Float_t df = (mField[j] - mField[i])*vcd->fSpread/vcd->fDistance*Ddt;
          delta[i] += df;
          delta[j] -= df;
        }
      }
      ++vcd;
    }
  }

  for (Int_t i=0; i<mNVerts; ++i)
  {
    mField[i] += delta[i];
    if (limit_df && mField[i] < 0) mField[i] = 0;
  }

  ColorizeTvor();
}

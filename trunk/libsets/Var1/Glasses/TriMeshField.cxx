// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TriMeshField.h"
#include <Glasses/TriMesh.h>
#include <Glasses/ParaSurf.h>
#include <Glasses/RGBAPalette.h>
#include "TriMeshField.c7"

#include <Opcode/Opcode.h>

#include <TMath.h>
#include <TRandom.h>
#include <TF3.h>

//==============================================================================
// TriMeshField
//==============================================================================

//__________________________________________________________________________
//
// mDim-dimensional field defined on vertices of a mesh.

ClassImp(TriMeshField);

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
  // Find min/max field values.

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
      TF3 tf3(GForm("TriMeshField_CT_%d", GetSaturnID()), mFormula.Data(), 0, 0);

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

void TriMeshField::UpdateMinMaxField(set<Int_t> vertices)
{
  // Iterate over specified vertices and update global min/max values.

  static const Exc_t _eh("TriMeshField::ParitallyFindMinMaxField ");

  if (mNVerts <= 0)
  {
    mMinValue = mMaxValue = 0;
  }
  else
  {
    set<Int_t>::iterator i   = vertices.begin();
    set<Int_t>::iterator end = vertices.end();

    if (mFormula.IsNull())
    {
      switch (mDim)
      {
        case 1:
          for ( ; i != end; ++i)
            check_min_max(std_val_1d(FVec(*i)));
          break;
        case 2:
          for ( ; i != end; ++i)
            check_min_max(std_val_2d(FVec(*i)));
          break;
        case 3:
          for ( ; i != end; ++i)
            check_min_max(std_val_3d(FVec(*i)));
          break;
        default:
          throw(_eh + "unsupported dimension.");
          break;
      }
    }
    else
    {
      TF3 tf3(GForm("TriMeshField_CT_%d", GetSaturnID()), mFormula.Data(), 0, 0);

      switch (mDim)
      {
        case 1:
          for ( ; i != end; ++i) {
            Float_t* F = FVec(*i);
            check_min_max((Float_t) tf3.Eval(F[0]));
          }
          break;
        case 2:
          for ( ; i != end; ++i) {
            Float_t* F = FVec(*i);
            check_min_max((Float_t) tf3.Eval(F[0], F[1]));
          }
          break;
        case 3:
          for ( ; i != end; ++i) {
            Float_t* F = FVec(*i);
            check_min_max((Float_t) tf3.Eval(F[0], F[1], F[2]));
          }
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
  UChar_t* C = TT.Cols();

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
    TF3 tf3(GForm("TriMeshField_CT_%d", GetSaturnID()), mFormula.Data(), 0, 0);

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

  UChar_t* C = TT.Cols();

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
    // !!! should cache formula as data-member.
    TF3 tf3(GForm("TriMeshField_CT_%d", GetSaturnID()), mFormula.Data(), 0, 0);

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

  // Regenerate changed triangles if necessary.
  if (regen_tring_cols && TT.HasTringCols())
  {
    set<Int_t> ct; // changed triangles
    for (set<Int_t>::iterator v = vertices.begin(); v != vertices.end(); ++v)
    {
      const TriMesh::VertexData & vd = mMesh->RefVDataVec()[*v];
      for (Int_t e = 0; e < vd.n_edges(); ++e)
      {
        const TriMesh::EdgeData& ed = mMesh->RefEDataVec()[vd.edge(e)];
        ct.insert(ed.fT1);
        ct.insert(ed.fT2); // Not needed for closed surfaces.
      }
    }
    if (*ct.begin() == -1) // Potentially remove no-tring entry.
      ct.erase(ct.begin());
    TT.GenerateTriangleColorsFromVertexColors(ct);
  }

  mMesh->StampReqTring(TriMesh::FID());
}

/**************************************************************************/

namespace {

struct GaussBlob { Float_t x, y, z, A, sgm, efc; };

struct GaussSprayer : public TriMesh::VertexVisitorMaxDist
{
  TriMeshField *mField;
  Float_t       mA, mExpFac;

  GaussSprayer(TriMesh* m, const Float_t origin[3], Float_t max_dist,
              TriMeshField* mf, Float_t a, Float_t exp_fac) :
    TriMesh::VertexVisitorMaxDist(m, origin, max_dist),
    mField (mf),
    mA     (a),
    mExpFac(exp_fac)
  {}
  virtual ~GaussSprayer() {}

  virtual Bool_t VisitVertex(Int_t vertex)
  {
    if (TriMesh::VertexVisitorMaxDist::VisitVertex(vertex))
    {
      mField->F(vertex) += mA * expf(mExpFac*mLastDistSqr);
      return kTRUE;
    }
    else
    {
      return kFALSE;
    }
  }
};

}

void TriMeshField::FillByGaussBlobs(Bool_t  reset_field, Int_t   n_blobs,
                                    Float_t A_min,       Float_t A_max,
                                    Float_t sigma_min,   Float_t sigma_max,
                                    Bool_t  minmax_p,    Bool_t  recolor_p)
{
  // Generate random 3D gaussian blobs on the surface of the mesh and
  // initialize field with the sum of contributions from all the blobs.
  //
  // gRandom is used for generation of the blob parameters.

  static const Exc_t _eh("TriMeshField::FillByXYGaussBlobs ");

  if (reset_field)
  {
    ResizeToMesh(1);
    SetField(0);
  }

  set<Int_t> all_changed;

  Float_t fgh[3], h_out;
  UInt_t   triangle = OPC_INVALID_ID;
  for (Int_t i=0; i<n_blobs; ++i)
  {
    GaussBlob B;
    B.A   = gRandom->Uniform(A_min, A_max);
    B.sgm = gRandom->Uniform(sigma_min, sigma_max);
    B.efc = -0.5f/(B.sgm*B.sgm);       // exp-factor
    //printf("Blob %3d: x,y,z=(%f,%f,%f); A,sgm=(%f,%f); efc=(%f)\n",
    //       i, B.x, B.y, B.z, B.A, B.sgm, B.efc);

    mMesh->GetParaSurf()->random_fgh(*gRandom, fgh);
    mMesh->FindPointFromFGH(fgh, false, &B.x, &h_out, &triangle);
    Int_t vertex = mMesh->FindClosestVertex(triangle, &B.x);

    GaussSprayer gs(*mMesh, &B.x, 3*B.sgm, this, B.A, B.efc);

    set<Int_t> visited;
    mMesh->VisitVertices(vertex, gs, visited, all_changed);
  }

  if (reset_field)
  {
    if (minmax_p)  FindMinMaxField();
    if (recolor_p) ColorizeTvor();
  }
  else
  {
    if (minmax_p)  UpdateMinMaxField(all_changed);
    if (recolor_p) PartiallyColorizeTvor(all_changed);
  }
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
  const vector<TriMesh::EdgeData>  &   edge_data_vec = mMesh->RefEDataVec();

  Float_t Ddt = diff_const*dt;

  // !!! This could be rewritten as loop over edges, now that wwe have them.
  for (Int_t i=0; i<mNVerts; ++i)
  {
    const TriMesh::VertexData&    vdata = vertex_data_vec[i];
    for (Int_t ei = 0; ei < vdata.n_edges(); ++ei)
    {
      const TriMesh::EdgeData& ed = edge_data_vec[vdata.edge(ei)];
      Int_t j = ed.other_vertex(i);

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
          Float_t df     = delta_f*ed.spread(i)/ed.distance();
          Float_t lim_df = TMath::Max(-mField[i] * ed.surface() / vdata.fSurface,
                                      0.5f * delta_f);
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
          Float_t df = (mField[j] - mField[i])*ed.spread(i)/ed.distance()*Ddt;
          delta[i] += df;
          delta[j] -= df;
        }
      }
    }
  }

  for (Int_t i=0; i<mNVerts; ++i)
  {
    mField[i] += delta[i];
    if (limit_df && mField[i] < 0) mField[i] = 0;
  }

  ColorizeTvor();
}

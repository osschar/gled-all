// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuRep.h"
#include "Tringula.h"
#include "TSPupilInfo.h"
#include "TriMeshLightField.h"
#include <Glasses/RGBAPalette.h>
#include "TringuRep.c7"

#include "TriMesh.h"

#include <TMath.h>

// TringuRep

//______________________________________________________________________________
//
// bTringStrips - will only be used if TringTvor has them.

ClassImp(TringuRep);

//==============================================================================

void TringuRep::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  bSmoothShading = false;
  bTringStrips   = false;

  mActionValue  = 1;
  mActionRadius = 1;
  mActionBorder = 0.5;

  bSprayDLWasOn = false;
}

TringuRep::TringuRep(const Text_t* n, const Text_t* t) :
  ZNode(n, t),
  mFieldMutex(GMutex::recursive)
{
  _init();
}

TringuRep::~TringuRep()
{}

//==============================================================================

void TringuRep::AssertVertexColorArray()
{
  static const Exc_t _eh("TringuRep::AssertVertexColorArray ");

  // If tringula is set, color arrays are initialized.
  assert_tringula(_eh);
}

UChar_t* TringuRep::GetVertexColorArray()
{
  return &mVertCols[0];
}

UChar_t* TringuRep::GetTriangleColorArray()
{
  return mTringCols.empty() ? 0 : &mTringCols[0];
}

void TringuRep::ColorArraysModified()
{
  StampReqTring(FID());
}

//==============================================================================

void TringuRep::SetTringula(Tringula* tring)
{
  static const Exc_t _eh("TringuRep::SetTringula ");

  if (!tring)
    throw _eh + "attempting to set to null.";

  set_link_or_die(mTringula.ref_link(), tring, FID());

  mTrans = tring->RefTrans();

  mVertCols.resize(4*tring->GetMesh()->GetTTvor()->mNVerts);

  update_triangle_color_array();
}

void TringuRep::SetField(TriMeshField* field)
{
  static const Exc_t _eh("TringuRep::SetField ");

  throw _eh + "should not be used -- call ActivateField() instead.";
}

void TringuRep::SetLightField(TriMeshLightField* lightfield)
{
  static const Exc_t _eh("TringuRep::SetLightField ");

  throw _eh + "should not be used -- call ActivateLightField() instead.";
}

void TringuRep::SetSmoothShading(Bool_t ss)
{
  bSmoothShading = ss;
  update_triangle_color_array();
  StampReqTring(FID());
}

void TringuRep::SwitchSmoothShading()
{
  SetSmoothShading(!bSmoothShading);
}

//==============================================================================

void TringuRep::update_triangle_color_array()
{
  if (bSmoothShading)
  {
    mTringCols.resize(0);
  }
  else
  {
    TringTvor &TT = * mTringula->GetMesh()->GetTTvor();
    if ((Int_t) mTringCols.size() != 4*TT.mNTrings)
      mTringCols.resize(4*TT.mNTrings);
    TT.GenerateTriangleColorsFromVertexColors(&mVertCols[0], &mTringCols[0]);
  }
}

void TringuRep::refresh_color_arrays()
{
  // Called after (de)activation of field or light-field.
  // Expects to run with mFieldMutex locked in a detached thread.

  if (*mField)
    mField->ApplyColors(*mLightField == 0);

  if (*mLightField)
    mLightField->ModulateColors(true);
}

void TringuRep::ActivateField(TriMeshField* field)
{
  // Set current field. This should be called via MIR and executed in a
  // detached thread as we also need to set the TriMeshColorArraySource of
  // previous and new field.

  static const Exc_t _eh("TringuRep::ActivateField ");

  suggest_MIR_presence(_eh, MC_IsDetached);

  GMutexHolder flck(mFieldMutex);

  TriMeshField *ex_field = *mField;

  if (field != ex_field)
  {
    if (ex_field)
    {
      GLensWriteHolder lck(ex_field);
      if (ex_field->GetColorArraySource() == this)
	ex_field->SetColorArraySource(0);
    }

    {
      GLensWriteHolder lck(this);
      set_link_or_die(mField.ref_link(), field, FID());
    }

    if (field)
    {
      GLensWriteHolder lck(field);
      field->SetColorArraySource(this);
    }
  }

  refresh_color_arrays();

  {
    GLensWriteHolder lck(this);
    mStampReqTring = Stamp(FID());
  }
}

void TringuRep::ActivateLightField(TriMeshLightField* lightfield)
{
  static const Exc_t _eh("TringuRep::ActivateLightField ");

  suggest_MIR_presence(_eh, MC_IsDetached);

  GMutexHolder flck(mFieldMutex);

  TriMeshField *ex_lightfield = *mLightField;

  if (lightfield != ex_lightfield)
  {
    if (ex_lightfield)
    {
      GLensWriteHolder lck(ex_lightfield);
      if (ex_lightfield->GetColorArraySource() == this)
	ex_lightfield->SetColorArraySource(0);
    }

    {
      GLensWriteHolder lck(this);
      set_link_or_die(mLightField.ref_link(), lightfield, FID());
    }

    if (lightfield)
    {
      GLensWriteHolder lck(lightfield);
      lightfield->SetColorArraySource(this);
    }
  }

  refresh_color_arrays();

  {
    GLensWriteHolder lck(this);
    mStampReqTring = Stamp(FID());
  }
}

void TringuRep::SwitchLightField(TriMeshLightField* lightfield)
{
  // Activate lightfield unless it is already active - then desactivate it.

  static const Exc_t _eh("TringuRep::ActivateLightField ");

  suggest_MIR_presence(_eh, MC_IsDetached);

  GMutexHolder flck(mFieldMutex);

  if (lightfield == *mLightField)
    ActivateLightField(0);
  else
    ActivateLightField(lightfield);
}

//==============================================================================

void TringuRep::ColorByTerrainProps(Int_t mode)
{
  // Colorize terrain mesh based on mode:
  // 0 - height
  // 1 - normal | up-vector

  static const Exc_t _eh("TringuCam::ColorByTerrainProps ");

  suggest_MIR_presence(_eh, MC_IsDetached);

  ActivateField(0);
  ActivateLightField(0);

  assert_palette(_eh);

  TriMesh* mesh = mTringula->GetMesh();
  switch (mode)
  {
    case 0:
      mesh->ColorByParaSurfCoord(*mPalette, this, 2);
      break;
    case 1:
      mesh->ColorByParaSurfNormal(*mPalette, this, 2, 0.3, 1);
      break;
    default:
      ISwarn(_eh + "Unsupported mode.");
  }
}


//==============================================================================
// Field adding / spraying
//==============================================================================

namespace
{
  struct FieldSprayer : public TriMesh::VertexVisitorMaxDist
  {
    TriMeshField *mField;
    Float_t       mValue;
    Float_t       mFullDist;
    Float_t       mFracFactor;

    FieldSprayer(TriMesh* m, const Float_t origin[3], Float_t max_dist,
		 Float_t full_dist, Float_t value, TriMeshField* field) :
      VertexVisitorMaxDist(m, origin, max_dist),
      mField      (field),
      mValue      (value),
      mFullDist   (full_dist),
      mFracFactor (1.0f / (max_dist - full_dist))
    {}
    virtual ~FieldSprayer() {}

    virtual Bool_t VisitVertex(Int_t vertex)
    {
      if (TriMesh::VertexVisitorMaxDist::VisitVertex(vertex))
      {
	Float_t dist   = TMath::Sqrt(mLastDistSqr);
	Float_t value  = mValue;
	if (dist > mFullDist)
	  value *= (1 - (dist - mFullDist))*mFracFactor;

	mField->F(vertex) += value;

	return kTRUE;
      }
      else
      {
	return kFALSE;
      }
    }

  };
}

void TringuRep::AddField(const Float_t point[3], Int_t vertex, Float_t factor)
{
  static const Exc_t _eh("TringuRep::AddField ");

  assert_field(_eh);

  TriMesh* mesh = mTringula->GetMesh();

  FieldSprayer sprayer(mesh, point, mActionRadius + mActionBorder,
                       mActionRadius, mActionValue*factor, *mField);
  set<Int_t>   vv, cv; // visited/changed vertices

  mesh->VisitVertices(vertex, sprayer, vv, cv);
  if (!cv.empty())
  {
    if (mLightField == 0)
    {
      mField->PartiallyApplyColors(cv, true);
    }
    else
    {
      mField->PartiallyApplyColors(cv, false);
      mLightField->PartiallyModulateColors(cv, true);
    }
  }
}

void TringuRep::BeginSprayField()
{
  if (bUseDispList)
  {
    bUseDispList  = false;
    bSprayDLWasOn = true;
  }

  StampReqTring(FID());
}

void TringuRep::EndSprayField()
{
  if (bSprayDLWasOn)
  {
    bUseDispList  = true;
    bSprayDLWasOn = false;
  }

  StampReqTring(FID());
}

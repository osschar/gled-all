// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringulaTester.h"
#include "Tringula.h"
#include "TSPupilInfo.h"
#include "TringulaTester.c7"

// TringulaTester

//______________________________________________________________________________
//
// Helper class to test functionality of Tringula and display the results.
//
// 1. Ray-terrain intersection
// 2. Detection of extendio intersection lines

ClassImp(TringulaTester);

//==============================================================================

void TringulaTester::_init()
{
  bRnrRay = false;
  mRayLen = 0;
  mRayPos.SetXYZ(0, 0, 0);
  mRayDir.SetXYZ(0, 0, 0);

  bRnrItsLines = false;
}

TringulaTester::TringulaTester(const Text_t* n, const Text_t* t) :
  ZNode(n, t)
{
  _init();
}

TringulaTester::~TringulaTester()
{}

//==============================================================================

void TringulaTester::SetRayVectors(const TVector3& pos, const TVector3& dir)
{
  mRayPos = pos;
  mRayDir = dir;
  Stamp(FID());
}

//==============================================================================

void TringulaTester::RayCollideTerrain()
{
  static const Exc_t _eh("TringulaTester::RayCollideTerrain ");

  assert_tringula(_eh);

  Opcode::Ray ray(&mRayPos[0], &mRayDir[0]);
  if (ray.mDir.NormalizeAndReport() == 0)
    throw _eh + "ray-direction vector is null.";

  Bool_t rnr_on = bRnrRay;
  {
    GLensReadHolder _lck(this);
    bRnrRay = false;
  }

  Bool_t status;
  {
    GLensReadHolder _lck(*mTringula);
    status = mTringula->RayCollide(ray, mRayLen, mRayColFaces);
  }

  printf("  status=%d, n faces = %d\n", status, mRayColFaces.GetNbFaces());
  for (UInt_t f=0; f<mRayColFaces.GetNbFaces(); ++f)
  {
    const Opcode::CollisionFace& cf = mRayColFaces.GetFaces()[f];
    printf("  %2d %6d  %10f  %10f %10f\n",
           f, cf.mFaceID, cf.mDistance, cf.mU, cf.mV);
  }

  {
    GLensReadHolder _lck(this);
    bRnrRay = rnr_on;
    StampReqTring(FID());
  }
}

//==============================================================================

void TringulaTester::FullBoxPrunning(Bool_t accumulate, Bool_t verbose)
{
  Bool_t rnr_on = bRnrItsLines;
  {
    GLensReadHolder _lck(this);
    bRnrItsLines = false;
  }

  mTringula->DoFullBoxPrunning(mItsLines, accumulate, verbose);

  {
    GLensReadHolder _lck(this);
    bRnrItsLines = rnr_on;
    StampReqTring(FID());
  }
}

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GLTesterOne
//
// Speed tests for GL operations. User can specify the number of outer and
// inner loops; the outer-loop timings are histogrammed with micro-second
// precision.
//
// Test(s) performed depend on value of mTestSelection variable:
//
// 1) TT_RnrAtom - point/line triangle rendering speed. Different modes
//    allow different ways of setting up the transformation matrix.
// 2) MatrixOps  - speed of retrieving matrices from GL.
//
// See also code of GLTesterOne_GL_Rnr.

#include "GLTesterOne.h"
#include "GLTesterOne.c7"

#include <Gled/GTime.h>

#include <TPad.h>

ClassImp(GLTesterOne)

/**************************************************************************/

void GLTesterOne::_init()
{
  mPupil = 0;

  mTestSelection = TT_RnrAtom;

  mRAtom  = RA_Point;
  mTMode  = TM_Vertex;
  mNSteps = 1000;
  mXMax   = 100;
  mTringU = 0.2;
  mTringV = 0.1;

  mMatrixOps = MO_GetFloat;

  bUseDispList = false;
  bPrint       = true;

  mNRedraws = 1000;
  mNTrial   = 20;

  h1TStat = new TH1F("TStat", "GlOne Time Statistics", 10, 0, 1);
}

/**************************************************************************/

void GLTesterOne::register_result(Double_t t)
{
  if(h1TStat != 0) {
    h1TStat->Fill(t);
  }
}

void GLTesterOne::RunTest()
{
  h1TStat->Reset();
  h1TStat->SetBins(200, 0, 1);
  
  for(int i=0; i<mNTrial; ++i) {
    mPupil->EmitImmediateRedrawRay();
  }
  while(h1TStat->GetEntries() < mNTrial) GTime::SleepMiliSec(10);

  Double_t m = h1TStat->GetMean(), r = h1TStat->GetRMS();
  Double_t min = TMath::Max(m - 5*r, 0.0);
  Double_t max = m + 5*r;
  Int_t nbins = TMath::Min(200, Int_t((max - min)/1e-6)); // Have mu-sec precision.

  h1TStat->Reset();
  h1TStat->SetBins(nbins, min, max);

  for(int i=0; i<mNRedraws; ++i) {
    mPupil->EmitImmediateRedrawRay();
  }
  while(h1TStat->GetEntries() < mNRedraws) GTime::SleepMiliSec(10);
  h1TStat->DrawCopy();
  gPad->Update();
}

/**************************************************************************/

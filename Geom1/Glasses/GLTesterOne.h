// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_GLTesterOne_H
#define Geom1_GLTesterOne_H

#include <Glasses/ZNode.h>
#include <Glasses/PupilInfo.h>

#include <TH1F.h>

class GLTesterOne : public ZNode {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(GLTesterOne);

public:
  enum TestType_e  { TT_RnrAtom, TT_MatrixOps };

  enum RnrAtom_e   { RA_Point, RA_Line, RA_Triangle };
  enum TransMode_e { TM_Vertex, TM_Translate, TM_PushPopMatrix };

  enum MatrixOps_e { MO_GetFloat, MO_GetDouble };

private:
  void _init();

protected:
  PupilInfo*	mPupil;		// X{gS} L{}

  TestType_e	mTestSelection; // X{GS} 7 PhonyEnum()

  // RnrAtom test parameters

  RnrAtom_e	mRAtom;		// X{GS} 7 PhonyEnum(-join=>1, -width=>8)
  TransMode_e	mTMode;		// X{GS} 7 PhonyEnum(-width=>8)

  Int_t		mNSteps;	// X{GS} 7 Value(-range=>[2,1e8,1], -join=>1)
  Float_t	mXMax;		// X{GS} 7 Value(-range=>[0,1000,1,1000])

  Float_t	mTringU;	// X{GS} 7 Value(-range=>[0,10,1,1000], -join=>1)
  Float_t	mTringV;	// X{GS} 7 Value(-range=>[0,10,1,1000])

  // MatrixOps test parameters

  MatrixOps_e	mMatrixOps;	// X{GS} 7 PhonyEnum()

  // Common parameters

  Bool_t	bUseDispList;	// X{GS} 7 Bool(-join=>1)
  Bool_t	bPrint;		// X{GS} 7 Bool()

  Int_t		mNRedraws;	// X{GS} 7 Value(-range=>[0,1e6,10], -join=>1)
  Int_t		mNTrial;	// X{GS} 7 Value(-range=>[0,1e6,10])

  TH1F*		h1TStat;	//! X{gs}

  void register_result(Double_t t);

public:
  GLTesterOne(const Text_t* n="GLTesterOne", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  void RunTest(); // X{Ed} 7 MButt()

#include "GLTesterOne.h7"
  ClassDef(GLTesterOne, 1) // Speed tests for some GL operations
}; // endclass GLTesterOne

GlassIODef(GLTesterOne);

#endif

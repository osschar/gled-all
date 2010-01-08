// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringulaTester_H
#define Var1_TringulaTester_H

#include <Glasses/ZNode.h>

#include <Opcode/Opcode.h>

#include <TVector3.h>

class Tringula;
class TSPupilInfo;

class TringulaTester : public ZNode
{
  MAC_RNR_FRIENDS(TringulaTester);

private:
  void _init();

protected:
  ZLink<Tringula>         mTringula;    // X{GS} L{a}

  Bool_t                  bRnrRay;      // X{GS}   7 Bool(-join=>1)
  Float_t                 mRayLen;      // X{GS}   7 Value(-range=>[0,10000,1,100])
  TVector3                mRayPos;      // X{GSRr} 7 Vector3()
  TVector3                mRayDir;      // X{GSRr} 7 Vector3()
  Opcode::CollisionFaces  mRayColFaces; //!

  Bool_t                  bRnrItsLines; // X{GS} 7 Bool() Intersection lines
  vector<Opcode::Segment> mItsLines;    //!

public:
  TringulaTester(const Text_t* n="TringulaTester", const Text_t* t=0);
  virtual ~TringulaTester();

  void SetRayVectors(const TVector3& pos, const TVector3& dir);

  void RayCollideTerrain(); // X{ED} 7 MButt()

  void FullBoxPrunning(Bool_t accumulate=false, Bool_t verbose=false);   // X{ED} 7 MCWButt()

#include "TringulaTester.h7"
  ClassDef(TringulaTester, 1);
}; // endclass TringulaTester

#endif

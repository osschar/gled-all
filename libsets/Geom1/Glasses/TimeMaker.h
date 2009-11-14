// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_TimeMaker_H
#define Geom1_TimeMaker_H

#include <Glasses/Operator.h>
#include <Stones/TimeMakerClient.h>

#include <TF1.h>

class TimeMaker : public Operator
{
  MAC_RNR_FRIENDS(TimeMaker);
public:
  enum WrapMode_e { WM_None, WM_Clip, WM_SawTooth, WM_Triangular, WM_SineWave };

private:
  void _init();

protected:
  Double_t   mFactor;   // X{GS} 7 Value(-join=>1)
  Double_t   mConstant; // X{GS} 7 Value()

  WrapMode_e mWrapMode; // X{GS} 7 PhonyEnum()
  Double_t   mMinT;     // X{GS} 7 Value(-join=>1)
  Double_t   mMaxT;     // X{GS} 7 Value()
  Double_t   mOmega;    // X{GS} 7 Value(-join=>1)
  Double_t   mDelta;    // X{GS} 7 Value()
  Double_t   mLastT;    // X{GS} 7 Value(-join=>1)
  Bool_t     bLastTOK;  // X{GS} 7 Bool()

  TString    mFormula;      // X{GS} Ray{Formula} 7 Textor()
  Bool_t     bApplyFormula; // X{GS} 7 Bool()
  TF1        mTF1;          //

  Bool_t     bPushToStack;  // X{GS} 7 Bool(-join=>1)
  Bool_t     bPopFromStack; // X{GS} 7 Bool()

  Bool_t     bEmitStamps;   // X{GS} 7 Bool(-join=>1)
  Bool_t     bPrintOut;     // X{GS} 7 Bool()

  ZLink<AList> mClients; // X{GS} L{}

public:
  TimeMaker(const Text_t* n="TimeMaker", const Text_t* t=0) :
    Operator(n,t) { _init(); }

  // Operator:
  virtual void ResetRecursively();
  virtual void Operate(Operator::Arg* op_arg);

  void AddClient(ZGlass* lens);    // X{E} C{1} 7 MCWButt()
  void RemoveClient(ZGlass* lens); // X{E} C{1} 7 MCWButt()

  void EmitFormulaRay();

#include "TimeMaker.h7"
  ClassDef(TimeMaker, 1);
}; // endclass TimeMaker


#endif

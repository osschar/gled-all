// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_PerfMeterOperator_H
#define GledCore_PerfMeterOperator_H

#include <Glasses/Operator.h>
#include <Glasses/PerfMeterTarget.h>

class SaturnInfo;

class PerfMeterOperator : public Operator {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(PerfMeterOperator);

public:
  enum	TestType_e { TT_Void=0, TT_Null, TT_IncCount, TT_SetVector };

private:
  void _init();

protected:
  void send_beam_or_flare(ZMIR& m);

  TestType_e		mTest;		// X{GS} 7 PhonyEnum(-join=>1);
  Bool_t		bUseBeams;	// X{GS} 7 Bool()

  Int_t			mVecSize;	// X{GS} 7 Value(-range=>[1,1000000,1,1])

  PerfMeterTarget*	mTarget;	// X{GS} L{}
  SaturnInfo*		mBeamHost;	// X{GS} L{}

public:
  PerfMeterOperator(const Text_t* n="PerfMeterOperator", const Text_t* t=0) : Operator(n,t) { _init(); }

  virtual void Operate(Operator::Arg* op_arg) throw(Operator::Exception);

#include "PerfMeterOperator.h7"
  ClassDef(PerfMeterOperator, 1)
}; // endclass PerfMeterOperator

GlassIODef(PerfMeterOperator);

#endif

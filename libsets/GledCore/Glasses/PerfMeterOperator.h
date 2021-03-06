// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_PerfMeterOperator_H
#define GledCore_PerfMeterOperator_H

#include <Glasses/Operator.h>
class PerfMeterTarget;
class SaturnInfo;

class PerfMeterOperator : public Operator
{
  MAC_RNR_FRIENDS(PerfMeterOperator);

public:
  enum	TestType_e { TT_Void=0, TT_Null, TT_IncCount, TT_SetVector };

private:
  void _init();

protected:
  void send_beam_or_flare(auto_ptr<ZMIR>& m);

  TestType_e		mTest;		// X{gS} 7 PhonyEnum(-join=>1);
  Bool_t		bUseBeams;	// X{gS} 7 Bool()

  Int_t			mVecSize;	// X{gS} 7 Value(-range=>[1,1000000,1,1])

  ZLink<PerfMeterTarget>	mTarget;	// X{gS} L{}
  ZLink<SaturnInfo>		mBeamHost;	// X{gS} L{}

public:
  PerfMeterOperator(const Text_t* n="PerfMeterOperator", const Text_t* t=0) : Operator(n,t) { _init(); }

  virtual void Operate(Operator::Arg* op_arg);

#include "PerfMeterOperator.h7"
  ClassDef(PerfMeterOperator, 1);
}; // endclass PerfMeterOperator


#endif

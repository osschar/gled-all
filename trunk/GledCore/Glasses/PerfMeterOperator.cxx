// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PerfMeterOperator
//
// If bUseBeams is true, make sure mBeamHost IS set.
// In fact should do this somwhere in routing code ... but anyway ...

#include "PerfMeterOperator.h"
#include "PerfMeterOperator.c7"

ClassImp(PerfMeterOperator)

/**************************************************************************/

void PerfMeterOperator::_init()
{
  mTest = TT_Void; bUseBeams = false;
  mVecSize  = 1;
  mTarget   = 0;
  mBeamHost = 0;
}

/**************************************************************************/

void PerfMeterOperator::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  Operator::PreOperate(op_arg);

  if(mTarget == 0) goto end_operate;

  switch(mTest) {
  case TT_Void: { break; }

  case TT_Null: {
    if(op_arg->fMultix) {
      mTarget->NullMethod();
    } else {
      auto_ptr<ZMIR> mir(mTarget->S_NullMethod());
      send_beam_or_flare(mir);
    }
    break;
  }

  case TT_IncCount: {
    if(op_arg->fMultix) {
      mTarget->IncCount();
    } else {
      auto_ptr<ZMIR> mir(mTarget->S_IncCount());
      send_beam_or_flare(mir);
    }
    break;
  }

  case TT_SetVector: {
    TVector vec(mVecSize);
    if(op_arg->fMultix) {
      mTarget->AssignVector(vec);
    } else {
      auto_ptr<ZMIR> mir(mTarget->S_AssignVector(vec));
      send_beam_or_flare(mir);
    }
    break;
  }

  }

 end_operate:
  Operator::PostOperate(op_arg);
}

/**************************************************************************/

void PerfMeterOperator::send_beam_or_flare(auto_ptr<ZMIR>& m)
{
  if(bUseBeams) {
    SaturnInfo *rec = mBeamHost.is_set() ? mBeamHost.get() : mSaturn->GetSaturnInfo();
    m->SetRecipient(rec);
  }
  mSaturn->PostMIR(m);
}

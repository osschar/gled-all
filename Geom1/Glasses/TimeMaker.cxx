// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TimeMaker
//
//

#include "TimeMaker.h"
#include "TimeMaker.c7"
#include <Glasses/Eventor.h>

ClassImp(TimeMaker);

/**************************************************************************/

void TimeMaker::_init()
{
  mFactor = 1;  mConstant = 0;

  mWrapMode = WM_None;
  mMinT = 0; mMaxT = 1;
  mLastT = 0; bLastTOK = false;

  bApplyFormula = false;

  bPushToStack = bPopFromStack = true;

  bEmitStamps = false;
}

/**************************************************************************/

void TimeMaker::EmitFormulaRay()
{
  mTF1.Compile(mFormula);
}

/**************************************************************************/

void TimeMaker::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  Operator::PreOperate(op_arg);

  Double_t t = mFactor*op_arg->fEventor->GetEventTime() + mConstant;

  if(mWrapMode != WM_None) {
    switch (mWrapMode) {
    case WM_Clip:
      if(t < mMinT)      t = mMinT;
      else if(t > mMaxT) t = mMaxT;
      break;
    case WM_SawTooth: {
      const Double_t D  = mMaxT - mMinT;
      const Double_t ft = (t - mMinT)/D;
      t = mMinT + D * (ft - TMath::Floor(ft));
      break;
    }
    case WM_Triangular: {
      const Double_t D  = 2*(mMaxT - mMinT);
      const Double_t ft = (t - mMinT)/D;
      t = mMinT + D * (ft - TMath::Floor(ft));
      if(t > mMaxT) t -= 2*(t - mMaxT);
      break;
    }
    default:
      break;
    }
  }

  Double_t dt;
  if(bApplyFormula) {
    Double_t t_pf = t;
    t = mTF1.Eval(t);
    dt = bLastTOK ? t - mLastT : 0;
    if(bPrintOut)
      printf("%s Operate t_in = %g, t_pf = %g, t = %g, dt = %g\n",
	     Identify().Data(), op_arg->fEventor->GetEventTime(), t_pf, t, dt);
  } else {
    dt = bLastTOK ? t - mLastT : 0;
    if(bPrintOut)
      printf("%s Operate t_in = %g, t = %g, dt = %g\n",
	     Identify().Data(), op_arg->fEventor->GetEventTime(), t, dt); 
  }

  if(mClients != 0) {
    list<TimeMakerClient*> clients;
    mClients->CopyListByGlass<TimeMakerClient>(clients);
    for(list<TimeMakerClient*>::iterator i=clients.begin(); i!=clients.end(); ++i)
      (*i)->TimeTick(t, dt);
  }

  if(bPushToStack)
    op_arg->fEventor->PushEventTime(t);

  Operator::PostOperate(op_arg);

  if(bPopFromStack)
    op_arg->fEventor->PopEventTime();

  mLastT   = t;
  bLastTOK = true;

  if(bEmitStamps) Stamp(FID());
}

/**************************************************************************/

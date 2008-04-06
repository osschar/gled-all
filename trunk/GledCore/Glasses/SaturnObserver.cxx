// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// SaturnObserver
//
//

#include "SaturnObserver.h"
#include "SaturnObserver.c7"

ClassImp(SaturnObserver)

/**************************************************************************/

void SaturnObserver::_init()
{
  mTarget = 0;
}

/**************************************************************************/

void SaturnObserver::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  Operator::PreOperate(op_arg);

  if(mTarget != 0) {
    auto_ptr<ZMIR> mir( mTarget->S_TellAverages() );
    mir->SetRecipient(mTarget.get());
    auto_ptr<ZMIR_RR> ret ( mSaturn->ShootMIRWaitResult(mir) );
    if(ret->BeamResult_OK()) {
      Float_t lavg[3];
      *ret >> lavg[0] >> lavg[1] >> lavg[2];
      printf("Got averages: %6.2f %6.2f %6.2f\n", lavg[0], lavg[1], lavg[2]);
    }
  }

  Operator::PostOperate(op_arg);
}

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PerfMeterTarget
//
//

#include "PerfMeterTarget.h"

ClassImp(PerfMeterTarget)

/**************************************************************************/

void PerfMeterTarget::_init()
{
  mCount = 0;
}

/**************************************************************************/

void PerfMeterTarget::AssignVector(TVector& vec)
{
  mVector.ResizeTo(vec);
  mVector = vec;
}

/**************************************************************************/

void PerfMeterTarget::NullMethod()
{
}

void PerfMeterTarget::IncCount()
{
  mExecMutex.Lock();
  ++mCount;
  Stamp(LibID(), ClassID());
  mExecMutex.Unlock();
}

/**************************************************************************/

#include "PerfMeterTarget.c7"

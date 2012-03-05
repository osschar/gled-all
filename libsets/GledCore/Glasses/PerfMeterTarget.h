// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_PerfMeterTarget_H
#define GledCore_PerfMeterTarget_H

#include <Glasses/ZList.h>

#include <TVector.h>

class PerfMeterTarget : public ZList
{
  MAC_RNR_FRIENDS(PerfMeterTarget);

private:
  void _init();

protected:
  UInt_t	mCount;		// X{GS} 7 Value()

  TVector	mVector;	// X{GRSQ}

public:
  PerfMeterTarget(const Text_t* n="PerfMeterTarget", const Text_t* t=0) : ZList(n,t) { _init(); }

  void AssignVector(TVector& vec); // X{E}

  void NullMethod();		// X{E}
  void IncCount();		// X{E}

#include "PerfMeterTarget.h7"
  ClassDef(PerfMeterTarget, 1);
}; // endclass PerfMeterTarget


#endif

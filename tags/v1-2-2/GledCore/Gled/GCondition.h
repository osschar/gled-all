// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GCondition_H
#define Gled_GCondition_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>
#ifndef __CINT__
#include <pthread.h>
#endif

class GCondition : public GMutex {
private:
#ifndef __CINT__
  pthread_cond_t	mCond;
#endif
public:
  GCondition(Init_e e=fast);
  ~GCondition();

  Int_t	Wait();
  Int_t TimedWait(UInt_t wait_ms);
  Int_t	Signal();
  Int_t Broadcast();

#include "GCondition.h7"
}; // endclass GCondition

#endif

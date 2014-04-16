// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GCondition_H
#define Gled_GCondition_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>
#include <Gled/GTime.h>
#ifndef __CINT__
#include <pthread.h>
#endif

class GCondition : public GMutex
{
private:
#ifndef __CINT__
  pthread_cond_t	mCond;
#endif
public:
  GCondition(Init_e e=fast);
  ~GCondition();

  Int_t	Wait();
  Int_t TimedWait(GTime time);
  Int_t TimedWaitUntil(GTime time);
  Int_t	Signal();
  Int_t Broadcast();
  Int_t	LockSignal();
  Int_t LockBroadcast();

#include "GCondition.h7"
  ClassDefNV(GCondition,0);
}; // endclass GCondition

#endif

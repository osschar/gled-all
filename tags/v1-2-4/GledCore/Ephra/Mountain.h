// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Mountain_H
#define Gled_Mountain_H

#include <Glasses/Eventor.h>
#include <Gled/GThread.h>
#include <Gled/GMutex.h>
#include <Gled/GCondition.h>
#include <Gled/GTime.h>

class Mountain;

struct DancerInfo {

  ZMirEmittingEntity* 	fOwner;

  GThread*		fThread;
  Eventor*		fEventor;
  Mountain*		fMountain;
  Operator::Arg* 	fOpArg;

  bool			fSuspended;
  bool			fSleeping;
  bool			fShouldSuspend;
  bool			fShouldExit;

  DancerInfo(ZMirEmittingEntity* o, GThread* t, Eventor* e, Mountain* m) :
    fOwner(o), fThread(t), fEventor(e), fMountain(m), fOpArg(0),
    fSuspended(false), fSleeping(false),
    fShouldSuspend(false), fShouldExit(false) {}
};

#ifndef __CINT__
typedef hash_map<Eventor*, DancerInfo*>			hEv2DI_t;
typedef hash_map<Eventor*, DancerInfo*>::iterator	hEv2DI_i;
#endif

class Mountain {
private:
  GMutex		hStageLock;
  GCondition		hSuspendCond;
  bool			bInSuspend;
  UInt_t		hSuspendCount;

protected:
#ifndef __CINT__
  hEv2DI_t		hOnStage;
#endif
  Saturn*		mSaturn;	// X{g}

  void stop_thread(DancerInfo* di);

public:
  Mountain(Saturn* s) : hStageLock(GMutex::recursive), mSaturn(s)
  { bInSuspend = false; }
  virtual ~Mountain() {}

  // Eventor request handling
  void Start(Eventor* e, bool suspend_immediately=false);
  void Stop(Eventor* e);
  void Suspend(Eventor* e);
  void Resume(Eventor* e);
  void Cancel(Eventor* e);

  void WipeThread(Eventor* e);

  // Suspension mechanism
  Int_t SuspendAll();
  void 	ResumeAll();
  void 	ConsiderSuspend(DancerInfo* di);

  // Shutdown ... kill all threads
  void Shutdown();

  static void  DancerCooler(DancerInfo* di);
  static void* DancerBeat(DancerInfo* di);

#include "Mountain.h7"
  ClassDef(Mountain, 0)
}; // endclass Mountain

#endif

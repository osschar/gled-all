// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Mountain.h"

#include <Glasses/Eventor.h>
#include <TSystem.h>

//#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

ClassImp(Mountain)

/**************************************************************************/
// Rhythm section ... static level
/**************************************************************************/

void Mountain::DancerCooler(DancerInfo* di)
{
  ISout(GForm("Mountain::DancerCooler thread exit for %s", di->fEventor->GetName()));
  di->fEventor->OnExit(di->fOpArg);
  delete di->fOpArg;
  di->fMountain->WipeThread(di->fEventor);
}

/**************************************************************************/

void* Mountain::DancerBeat(DancerInfo* di)
{
  GThread::SetCancelState(GThread::CS_Enable);
  GThread::SetCancelType(GThread::CT_Async);
  //GThread::SetCancelType(GThread::CT_Deferred);

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, GThread::SigCONT);
  sigaddset(&set, GThread::SigSTOP);
  pthread_sigmask(SIG_SETMASK, &set, &oldset);

  di->fStartTime = gSystem->Now();
  Operator::Arg* op_arg = di->fEventor->PreDance();
  if(op_arg == 0) GThread::Exit();

  GTHREAD_CU_PUSH;

  di->fOpArg = op_arg;
  di->fEventor->OnStart(op_arg);

  bool exc_p = false, exit_p = false, suspend_p = false;
  TTime op_start, op_stop, now;

  if(di->fShouldSuspend) {
    // Hack: allows connecting moons to properly initialize multix
    // suspended threads.
    // Time difference is wrong, but this shouldn't matter for multixen.
    op_start = gSystem->Now() - TTime(di->fEventor->GetInterBeatMS() / 2);
    goto suspend_exit_check;
  }

  while(1) {
    exc_p = exit_p = suspend_p = false;

    op_start = gSystem->Now();
    try {
      di->fEventor->PreBeat(op_arg);
      di->fEventor->Operate(op_arg);
      di->fEventor->PostBeat(op_arg);
    }
    catch(Operator::Exception op_exc) {
      exc_p = true;

      switch(op_exc.fExc) {

      case Operator::OE_Done:
	di->fEventor->PostDance(op_arg);
	exit_p = true;

      case Operator::OE_Continue:
	di->fEventor->OnContinue(op_arg, op_exc);
	break;

      case Operator::OE_Wait:
	di->fEventor->OnWait(op_arg, op_exc);
	suspend_p = true;
	break;

      case Operator::OE_Stop:
	di->fEventor->OnStop(op_arg, op_exc);
	exit_p = true;
	break;

      case Operator::OE_Break:
	di->fEventor->OnBreak(op_arg, op_exc);
	exit_p = true;
	break;

      }

    }
    if(!exc_p && !op_arg->fContinuous) {
      di->fEventor->PostDance(op_arg);
      exit_p = true;
    }
    op_stop = gSystem->Now();

    di->fLastDuration = op_start - op_stop;
    if(di->fAvg10Duration == 0) {
      di->fAvg10Duration = ULong_t(di->fLastDuration);
    } else {
      di->fAvg10Duration = 0.9*di->fAvg10Duration +
	0.1*ULong_t(di->fLastDuration);
    }

  suspend_exit_check:
    di->fOpArg->fSuspendidor.Lock();
    if(exit_p || di->fShouldExit) {
      di->fOpArg->fSuspendidor.Unlock();
      GThread::Exit();
    }

    if(suspend_p || di->fShouldSuspend) {
      di->fEventor->OnSuspend(op_arg);
      di->fSuspended = true;
      if(op_arg->fSignalSafe) {
	di->fOpArg->fSuspendidor.Unlock();
	di->fThread->Kill(GThread::SigSTOP);
      } else {
	di->fOpArg->fSuspendidor.Wait();
	di->fOpArg->fSuspendidor.Unlock();
      }
      // Check again for exit during suspend
      if(di->fShouldExit) {
	GThread::Exit();
      }
    } else {
      di->fOpArg->fSuspendidor.Unlock();
    }

    now = gSystem->Now();
    Int_t sleep_time = di->fEventor->GetInterBeatMS() - (ULong_t)(now - op_start);
    if(sleep_time > 0) {
      if(op_arg->fSignalSafe) {
	gSystem->Sleep(UInt_t(sleep_time));
      } else {
	di->fOpArg->fSuspendidor.Lock();
	di->fSleeping = true;
	int timed_out = di->fOpArg->fSuspendidor.TimedWait(sleep_time);
	di->fSleeping = false;
	di->fOpArg->fSuspendidor.Unlock();
	if(!timed_out) {
	  goto suspend_exit_check;
	}
      }
    }
  } // end forever

  GTHREAD_CU_POP;
}

/**************************************************************************/
// Rhythm section ... Mountain<->Eventor interface
/**************************************************************************/

void Mountain::Start(Eventor* e, bool suspend_immediately)
{
  if(bInSuspend) { // !!! should use mutex ... rethink suspend all !!!
    // In fact ... should add it to on stage w/ fThread=0 ...
    ISerr("Mountain::Start ... Mountain suspended ... stalling");
    return;
  }
  
  hStageLock.Lock();
  if(hOnStage.find(e) != hOnStage.end()) {
    hStageLock.Unlock();
    ISerr(GForm("Mountain::Start ... Thread of %s already alive",
		e->GetName()));
    return;
  }

  GThread* t = new GThread((thread_f)DancerBeat);
  DancerInfo* di = new DancerInfo(t, e, this);
  t->SetArg((void*)di);
  hOnStage[e] = di;
  hStageLock.Unlock();

  t->SetEndFoo((thread_cu_f)DancerCooler);
  t->SetEndArg((void*)di);
  if(suspend_immediately) di->fShouldSuspend = true;
  t->Spawn();
}

void Mountain::Stop(Eventor* e)
{
  hStageLock.Lock();
  hEv2DI_i i = hOnStage.find(e);
  if(i == hOnStage.end() || i->second==0) {
    hStageLock.Unlock();
    ISerr(GForm("Mountain::GimmeSilence ... can't find thread of %s",
		e->GetName()));
    return;
  }
  DancerInfo* di = i->second;
  hStageLock.Unlock();

  di->fOpArg->fSuspendidor.Lock();
  if(di->fOpArg->fSignalSafe) {
    if(di->fSuspended) {
      di->fShouldExit = true;
      di->fThread->Kill(GThread::SigCONT);
    } else {
      di->fOpArg->fSignalodor.Lock();
      di->fThread->Cancel();
      di->fOpArg->fSignalodor.Unlock();
    }
  } else {
    di->fShouldExit = true;
    if(di->fSuspended || di->fSleeping) {
      di->fOpArg->fSuspendidor.Signal();
    }
  }
  di->fOpArg->fSuspendidor.Unlock();
}

void Mountain::Suspend(Eventor* e)
{
  hStageLock.Lock();
  hEv2DI_i i = hOnStage.find(e);
  if(i == hOnStage.end() || i->second==0) {
    hStageLock.Unlock();
    ISerr(GForm("Mountain::Suspend ... can't find thread of %s",
		e->GetName()));
    return;
  }
  hStageLock.Unlock();

  DancerInfo* di = i->second;
  di->fOpArg->fSuspendidor.Lock();
  if(di->fOpArg->fSignalSafe) {
    di->fOpArg->fSignalodor.Lock();
    di->fThread->Kill(GThread::SigSTOP);
    di->fOpArg->fSignalodor.Unlock();
    di->fEventor->OnSuspend(di->fOpArg);
    di->fSuspended = true;
  } else {
    di->fShouldSuspend = true;
    if(di->fSleeping) {
      di->fOpArg->fSuspendidor.Signal();
    }
  }
  di->fOpArg->fSuspendidor.Unlock();
}

void Mountain::Resume(Eventor* e)
{
  hStageLock.Lock();
  hEv2DI_i i = hOnStage.find(e);
  if(i == hOnStage.end() || i->second==0) {
    hStageLock.Unlock();
    ISerr(GForm("Mountain::Resume ... can't find thread of %s",
		e->GetName()));
    return;
  }
  hStageLock.Unlock();

  DancerInfo* di = i->second;
  di->fOpArg->fSuspendidor.Lock();
  di->fSuspended = false;
  di->fShouldSuspend = false;
  di->fEventor->OnResume(di->fOpArg);
  if(di->fOpArg->fSignalSafe) {
    di->fThread->Kill(GThread::SigCONT);
  } else {
    di->fOpArg->fSuspendidor.Signal();
  }
  di->fOpArg->fSuspendidor.Unlock();
}

/**************************************************************************/

void Mountain::WipeThread(Eventor* e)
{
  hStageLock.Lock();
  hEv2DI_i i = hOnStage.find(e);
  if(i == hOnStage.end() || i->second==0) {
   hStageLock.Unlock();
   ISerr(GForm("Mountain::WipeThread ... thread of %s not found", e->GetName()));
    return;
  }
  delete i->second->fThread;
  delete i->second;
  hOnStage.erase(i);  
  hStageLock.Unlock();
}

/**************************************************************************/
// Suspending
/**************************************************************************/

Int_t Mountain::SuspendAll()
{
  // Tries to suspend all operators ...
  // fails miserably if some threads have longer periods ... should timeout!!!!
  //
  // Also ... this is not at all appropriate.
  // Should have per-queen list of threads ... ie dancers.
  // For each of them should also know in what way it affects other structures.

  hSuspendCond.Lock(); // unlocked in resume all
  hSuspendCount = 0;
  bInSuspend = true;
  UInt_t n;
  hStageLock.Lock(); n = hOnStage.size(); hStageLock.Unlock();
  if(n==0) {
    //hSuspendCond.Unlock();
    return 0;
  }
  do {
    int ws = hSuspendCond.TimedWait(5);
    if(ws==0) hSuspendCount++;
    hStageLock.Lock(); n = hOnStage.size(); hStageLock.Unlock();
    if(ws && hSuspendCount < n) {
      ISerr("Mountain::SuspendAll timed out ... stalling");
      //hSuspendCond.Unlock();
      return 1;
    }
  } while(hSuspendCount < n);
  ISmess(GForm("Mountain::SuspendAll success ... %d threads SIGSUSP-ed",
	       hSuspendCount));
  //hSuspendCond.Unlock();
  return 0;
}

void Mountain::ResumeAll()
{
  if(bInSuspend==false) return;
  bInSuspend = false;
  hStageLock.Lock();
  for(hEv2DI_i i=hOnStage.begin(); i!=hOnStage.end(); i++) {
    hSuspendCount--;
    if(i->second->fSuspended) {
      ISmess(GForm("Mountain::RestartAll restarting thread of %s",
		   i->second->fEventor->GetName()));
      i->second->fSuspended = false;
      i->second->fThread->Kill(GThread::SigCONT);
    }
  }
  hStageLock.Unlock();
  hSuspendCond.Unlock();
}

void Mountain::ConsiderSuspend(DancerInfo* di)
{
  // Called from each DancerBeat ... like offering the mountain to suspend it

  if(bInSuspend) {
    hSuspendCond.Lock(); hSuspendCond.Signal(); hSuspendCond.Unlock();
    ISmess(GForm("Mountain::ConsiderSuspend suspending thread of %s",
		 di->fEventor->GetName()));
    di->fSuspended = true;
    di->fThread->Kill(GThread::SigSTOP);
  }
}

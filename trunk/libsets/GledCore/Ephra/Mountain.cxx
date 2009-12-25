// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Mountain.h"

#include <Glasses/Eventor.h>
#include <TSystem.h>

//#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#ifdef __APPLE__
#define _XOPEN_SOURCE
#endif
#include <ucontext.h>
#include <fenv.h> // requires _GNU_SOURCE to be defined for trap control

class MountainThread : public GThread
{
public:
  MountainThread(const Text_t* name, GThread_foo foo, void* arg=0, bool detached=false) :
    GThread(name, foo, arg, detached),
    fTerminalSignalId(0)
  {}
  virtual ~MountainThread() {}

  ucontext_t   fTerminalSignalRetourContext;
  int          fTerminalSignalId;
};

ClassImp(Mountain);

/**************************************************************************/
// Rhythm section ... static level
/**************************************************************************/

void Mountain::DancerCooler(DancerInfo* di)
{
  static const Exc_t _eh("Mountain::DancerCooler ");

  ISdebug(1, _eh + GForm("thread exit for %s", di->fEventor->GetName()));

  di->fOpArg->fStop.SetNow();

  GLensWriteHolder wrlck(di->fEventor);

  DancerInfo* rdi = di->fMountain->UnregisterThread(di->fEventor);
  if (di != rdi)
  {
    ISerr(_eh + "mismatch between local and global thread data.");
  }
  di->fEventor->OnExit(di->fOpArg);
  delete di->fOpArg;
  delete di;
}

/**************************************************************************/

namespace
{
  void sh_DancerSuspender(int sig)
  {
    int recsig;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, GThread::SigUSR2);
    sigwait(&set, &recsig);
  }

  void sh_TerminalSigHandler(int sid, siginfo_t* sinfo, void* sfoo)
  {
    // ucontext_t *sctx = (ucontext_t*) sfoo;

    fprintf(stderr, "Terminal signal handler entered ...\n");

    MountainThread* mt = (MountainThread*) GThread::Self();
    fprintf(stderr, "thread id=%d name='%s' state=%s.\n", mt->GetIndex(), mt->GetName(),
	    GThread::RunningStateName(mt->GetRunningState()));

    switch (sid)
    {
      case SIGILL:
	fprintf(stderr, "Illegal instruction.\n");
	break;
      case SIGBUS:
	fprintf(stderr, "Bus error.\n");
	break;
      case SIGSEGV:
	fprintf(stderr, "Segmentation violation.\n");
	break;
      case SIGFPE:
	fprintf(stderr, "Floating-point exception.\n");
	// The bits are not set ... seems trap setting overrides it.
	// {
	//   int foo = fetestexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
	//   printf(" FE: DIVBYZERO=%d, INVALID=%d, OVERFLOW=%d\n", foo & FE_DIVBYZERO, foo & FE_INVALID, foo & FE_OVERFLOW);
	// }
	// Clear excepts - no need.
	break;

      default:
	fprintf(stderr, "Unexpected signal %d\n", sid);
	break;
    }

    gSystem->StackTrace();

    mt->fTerminalSignalId = sid;

    if (setcontext(&mt->fTerminalSignalRetourContext))
    {
      perror("setcontext failed in signal handler ... not good:");
    }
  }
}

void* Mountain::DancerBeat(DancerInfo* di)
{
  static const Exc_t _eh("Mountain::DancerBeat ");

  void* err_ret = (void*) 1;

  GThread::SetCancelState(GThread::CS_Enable);
  GThread::SetCancelType(GThread::CT_Async);
  //GThread::SetCancelType(GThread::CT_Deferred);

  { // Signal handle init; only used for SignalSafe threads
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, GThread::SigUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, 0);

    sigemptyset(&set);
    sigaddset(&set, GThread::SigUSR2);
    pthread_sigmask(SIG_BLOCK, &set, 0);

    struct sigaction sac;

    sac.sa_handler = sh_DancerSuspender;
    sigemptyset(&sac.sa_mask);
    sac.sa_flags = 0;
    sigaction(SIGUSR1, &sac, 0);
  }

  Operator::Arg* op_arg;
  {
    GLensWriteHolder wrlck(di->fEventor);
    op_arg = di->fEventor->PreDance();
  }
  if (op_arg == 0) return err_ret;

  di->fThread->SetEndFoo((GThread_cu_foo)DancerCooler);
  di->fThread->SetEndArg((void*)di);

  di->fOpArg = op_arg;
  op_arg->fStart.SetNow();
  op_arg->fBeatID = -1;
  {
    GLensWriteHolder wrlck(di->fEventor);
    di->fEventor->OnStart(op_arg);
  }

  {
    sigset_t set;
    sigemptyset(&set);

    struct sigaction sac;

    if (di->fEventor->GetTrapILL())
    {
      sac.sa_sigaction = sh_TerminalSigHandler;
      sigemptyset(&sac.sa_mask);
      sac.sa_flags = 0;
      sigaction(SIGILL, &sac, 0);
      sigaddset(&set, SIGILL);
    }

    if (di->fEventor->GetTrapBUS())
    {
      sac.sa_sigaction = sh_TerminalSigHandler;
      sigemptyset(&sac.sa_mask);
      sac.sa_flags = 0;
      sigaction(SIGBUS, &sac, 0);
      sigaddset(&set, SIGBUS);
    }

    if (di->fEventor->GetTrapSEGV())
    {
      sac.sa_sigaction = sh_TerminalSigHandler;
      sigemptyset(&sac.sa_mask);
      sac.sa_flags = 0;
      sigaction(SIGSEGV, &sac, 0);
      sigaddset(&set, SIGSEGV);
    }

    if (di->fEventor->GetTrapFPE())
    {
      sac.sa_sigaction = sh_TerminalSigHandler;
      sigemptyset(&sac.sa_mask);
      sac.sa_flags = 0;
      sigaction(SIGFPE, &sac, 0);
      sigaddset(&set, SIGFPE);

      // This is platform dependant.
#ifdef __APPLE__
      fenv_t fe;
      fegetenv(&fe);
#if (defined(__ppc__) || defined(__ppc64__))
      fe |= (FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#elif (defined (__i386__) || defined( __x86_64__ ))
      fe.__control |= (FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
      fe.__mxcsr   |= (FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#else
#error Unknown architecture
#endif
      fesetenv(&fe);
#else
      feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
    }

    pthread_sigmask(SIG_UNBLOCK, &set, 0);


    MountainThread *mt = (MountainThread*) di->fThread;

    if (getcontext(&mt->fTerminalSignalRetourContext))
    {
      perror("getcontext failed:");
      return err_ret;
    }

    if (mt->fTerminalSignalId != 0)
    {
      printf("Strange strange -- who was dead now walks again. And the number of his cross was %d.\n", mt->fTerminalSignalId);
      int foo = fetestexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
      printf(" FE: DIVBYZERO=%d, INVALID=%d, OVERFLOW=%d\n", foo & FE_DIVBYZERO, foo & FE_INVALID, foo & FE_OVERFLOW);
      di->fEventor->OnTerminalSignal(op_arg, mt->fTerminalSignalId);
      return err_ret;
    }
  }

  bool exc_p = false, exit_p = false, suspend_p = false;

  if (di->fShouldSuspend)
  {
    // Hack: allows connecting moons to properly initialize multix
    // suspended threads.
    // Time difference is wrong, but this shouldn't matter for multixen.
    op_arg->fBeatStart.SetNow();
    op_arg->fBeatStart -= (Long_t)(1000 * di->fEventor->GetInterBeatMS() / 2);
    goto suspend_exit_check;
  }

  while (true)
  {
    exc_p = exit_p = suspend_p = false;

    op_arg->fBeatStart.SetNow();
    ++op_arg->fBeatID;
    try
    {
      {
	GLensWriteHolder wrlck(di->fEventor);
	di->fEventor->PreBeat(op_arg);
      }
      // Operate this the only method NOT called with write lock.
      di->fEventor->Operate(op_arg);
      {
	GLensWriteHolder wrlck(di->fEventor);
	di->fEventor->PostBeat(op_arg);
      }
    }
    catch (Operator::Exception& op_exc)
    {
      exc_p = true;

      switch (op_exc.fExc)
      {
        case Operator::OE_Done:
        {
          GLensWriteHolder wrlck(di->fEventor);
          di->fEventor->PostDance(op_arg);
          exit_p = true;
          break;
        }
        case Operator::OE_Continue:
        {
          GLensWriteHolder wrlck(di->fEventor);
          di->fEventor->OnContinue(op_arg, op_exc);
          break;
        }
        case Operator::OE_Wait:
        {
          GLensWriteHolder wrlck(di->fEventor);
          di->fEventor->OnWait(op_arg, op_exc);
          suspend_p = true;
          break;
        }
        case Operator::OE_Stop:
        {
          GLensWriteHolder wrlck(di->fEventor);
          di->fEventor->OnStop(op_arg, op_exc);
          exit_p = true;
          break;
        }
        case Operator::OE_Break:
        {
          ISerr(_eh + GForm("[%s] Exit with Break", di->fEventor->GetName()));
          GLensWriteHolder wrlck(di->fEventor);
          di->fEventor->OnBreak(op_arg, op_exc);
          exit_p = true;
          break;
        }
      }
    }
    catch (Exc_t& exc)
    {
      exc_p = true;
      ISerr(_eh + GForm("[%s] Unhandled exception, breaking event loop, Exc_t: '%s'",
                        di->fEventor->GetName(), exc.Data()));
      GLensWriteHolder wrlck(di->fEventor);
      di->fEventor->OnBreak(op_arg, exc);
      exit_p = true;
    }
    catch (exception& exc)
    {
      exc_p = true;
      ISerr(_eh + GForm("[%s] Unhandled exception, breaking event loop, std::exception: '%s'",
                        di->fEventor->GetName(), exc.what()));
      GLensWriteHolder wrlck(di->fEventor);
      di->fEventor->OnBreak(op_arg, exc.what());
      exit_p = true;
    }
    catch (...)
    {
      exc_p = true;
      ISerr(_eh + GForm("[%s] Unhandled unknown exception, breaking event loop.",
                        di->fEventor->GetName()));
      GLensWriteHolder wrlck(di->fEventor);
      di->fEventor->OnBreak(op_arg, "");
      exit_p = true;
      throw;
    }

    op_arg->fBeatStop.SetNow();
    op_arg->fBeatSum += op_arg->fBeatStop - op_arg->fBeatStart;
    if (!exc_p && !op_arg->fContinuous)
    {
      GLensWriteHolder wrlck(di->fEventor);
      di->fEventor->PostDance(op_arg);
      exit_p = true;
    }

  suspend_exit_check:
    di->fOpArg->fSuspendidor.Lock();
    if (exit_p || di->fShouldExit)
    {
      di->fOpArg->fSuspendidor.Unlock();
      return 0;
    }

    if (suspend_p || di->fShouldSuspend)
    {
      di->fEventor->OnSuspend(op_arg);
      di->fSuspended = true;
      if (op_arg->fSignalSafe)
      {
	di->fOpArg->fSuspendidor.Unlock();
	di->fThread->Kill(GThread::SigUSR1);
      } else {
	di->fOpArg->fSuspendidor.Wait();
	di->fOpArg->fSuspendidor.Unlock();
      }
      // Check again for exit during suspend
      if (di->fShouldExit)
      {
	return 0;
      }
    }
    else
    {
      di->fOpArg->fSuspendidor.Unlock();
    }

    GTime since_start(GTime::I_Now);
    since_start -= op_arg->fBeatStart;

    Int_t sleep_time = di->fEventor->GetInterBeatMS() - since_start.ToMiliSec();
    if (sleep_time > 0)
    {
      if (op_arg->fSignalSafe)
      {
	gSystem->Sleep(UInt_t(sleep_time));
      }
      else
      {
	di->fOpArg->fSuspendidor.Lock();
	di->fSleeping = true;
	int timed_out = di->fOpArg->fSuspendidor.TimedWaitMS(sleep_time);
	di->fSleeping = false;
	di->fOpArg->fSuspendidor.Unlock();
	if (!timed_out)
        {
	  goto suspend_exit_check;
	}
      }
    }
  } // end forever

  ISerr(_eh + "Got to the end of function ... not expected to end here.");
  return err_ret;
}

/**************************************************************************/
// Rhythm section ... Mountain<->Eventor interface
/**************************************************************************/

void Mountain::Start(Eventor* e, bool suspend_immediately)
{
  static const Exc_t _eh("Mountain::Start ");

  if (bInSuspend)
  {
    // !!! should use mutex ... rethink suspend all !!!
    // In fact ... should add it to on stage w/ fThread=0 ...
    ISerr(_eh + "Mountain suspended, stalling.");
    return;
  }

  hStageLock.Lock();
  if(hOnStage.find(e) != hOnStage.end()) {
    hStageLock.Unlock();
    ISerr(_eh + "Thread of " + e->GetName() + " already alive.");
    return;
  }

  GThread* t = new MountainThread("Mountain-DancerBeat",
				  (GThread_foo) DancerBeat, 0,
				  true);
  DancerInfo* di = new DancerInfo(t, e, this);
  hOnStage[e] = di;
  t->SetStartArg((void*) di);
  hStageLock.Unlock();

  if (suspend_immediately) di->fShouldSuspend = true;
  t->Spawn();
}

/**************************************************************************/

void Mountain::stop_thread(DancerInfo* di)
{
  di->fOpArg->fSuspendidor.Lock();
  if(di->fOpArg->fSignalSafe) {
    if(di->fSuspended) {
      di->fShouldExit = true;
      di->fThread->Kill(GThread::SigUSR2);
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

void Mountain::Stop(Eventor* e)
{
  hStageLock.Lock();
  hEv2DI_i i = hOnStage.find(e);
  if(i == hOnStage.end() || i->second==0) {
    hStageLock.Unlock();
    ISerr(GForm("Mountain::Stop ... can't find thread of %s",
		e->GetName()));
    return;
  }
  DancerInfo* di = i->second;
  hStageLock.Unlock();

  stop_thread(di);
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
    di->fThread->Kill(GThread::SigUSR1);
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
    di->fThread->Kill(GThread::SigUSR2);
  } else {
    di->fOpArg->fSuspendidor.Signal();
  }
  di->fOpArg->fSuspendidor.Unlock();
}

void Mountain::Cancel(Eventor* e)
{
  hStageLock.Lock();
  hEv2DI_i i = hOnStage.find(e);
  if(i == hOnStage.end() || i->second==0) {
    hStageLock.Unlock();
    ISerr(GForm("Mountain::Cancel ... can't find thread of %s",
		e->GetName()));
    return;
  }
  DancerInfo* di = i->second;
  hStageLock.Unlock();

  di->fThread->Cancel();
}

/**************************************************************************/

DancerInfo* Mountain::UnregisterThread(Eventor* e)
{
  static const Exc_t _eh("Mountain::UnregisterThread ");

  GMutexHolder stg_lck(hStageLock);
  hEv2DI_i i = hOnStage.find(e);
  if(i == hOnStage.end() || i->second==0) {
    ISerr(_eh + GForm("thread of %s not found.", e->GetName()));
    return 0;
  }
  DancerInfo* ret = i->second;
  hOnStage.erase(i);
  return ret;
}

void Mountain::WipeThread(Eventor* e)
{
  DancerInfo* di = UnregisterThread(e);
  delete di;
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
    int ws = hSuspendCond.TimedWaitMS(5);
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
      i->second->fThread->Kill(GThread::SigUSR2);
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
    di->fThread->Kill(GThread::SigUSR1);
  }
}

/**************************************************************************/

void Mountain::Shutdown()
{
  // Called from Saturn on shutdown. Kills all threads.

  hStageLock.Lock();
  if(hOnStage.size() == 0) {
    hStageLock.Unlock();
    return;
  }
  for(hEv2DI_i i=hOnStage.begin(); i!=hOnStage.end(); ++i) {
    stop_thread(i->second);
  }
  hStageLock.Unlock();

  int  count = 0;
  while(1) {
    gSystem->Sleep(100);
    hStageLock.Lock();
    int n = hOnStage.size();
    if(n > 0 && count > 10) {
      for(hEv2DI_i i=hOnStage.begin(); i!=hOnStage.end(); ++i) {
	i->second->fThread->Cancel();
      }
    }
    hStageLock.Unlock();
    if(n == 0) break;
    ISmess(GForm("Mountain::Shutdown waiting for %d threads", n));
    ++count;
  }
  ISmess("Mountain::Shutdown all threads stopped");
}

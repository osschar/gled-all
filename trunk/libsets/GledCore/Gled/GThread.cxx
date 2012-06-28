// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GThread.h"
#include <Gled/Gled.h>
#include <Glasses/ZMirEmittingEntity.h>

#include "TThread.h"
#include "TSystem.h"
#include "TUnixSystem.h"

#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __APPLE__
#define _XOPEN_SOURCE
#endif
#include <ucontext.h>
#include <fenv.h> // requires _GNU_SOURCE to be defined for trap control

//______________________________________________________________________________
//
// POSIX thread wrapper class - Gled specific as it provides
// data members required for authorization and MIR processing.
//

ClassImp(GThread);

GThread*                 GThread::sMainThread   = 0;
int                      GThread::sThreadCount  = 0;
map<pthread_t, GThread*> GThread::sThreadMap;
list<GThread*>           GThread::sThreadList;
GMutex                   GThread::sContainerLock;
GMutex                   GThread::sDetachCtrlLock(GMutex::recursive);
int                      GThread::sMinStackSize = 0;

pthread_key_t  GThread::TSD_Self;
GThread       *GThread::sInvalidPtr = (GThread*) 0x1l;
int            GThread::sSigMap[SigMAX] =
{ -1,
  SIGHUP,  SIGINT,    SIGQUIT, SIGILL,   SIGTRAP, SIGABRT, SIGBUS,  SIGFPE,
  SIGKILL, SIGUSR1,   SIGSEGV, SIGUSR2,  SIGPIPE, SIGALRM, SIGTERM, -1,
  SIGCHLD, SIGCONT,   SIGSTOP, SIGTSTP,  SIGTTIN, SIGTTOU, SIGURG,  SIGXCPU,
  SIGXFSZ, SIGVTALRM, SIGPROF, SIGWINCH, SIGIO,   -1,      SIGSYS
};

//==============================================================================

GThread::GThread(const Text_t* name) :
  mRunningState (RS_Running),
  mIndex        (-1),
  mThreadListIt (),
  mId           (0),
  mRootTThread  (0),

  mName     (name),
  mStartFoo (0), mStartArg (0),
  bDetached (false), bDetachOnExit (false),
  mNice     (0),
  mStackSize(0),

  mSigHandlerDefault(0), mSigHandlerVector(SigMAX),
  mTerminalContext(new ucontext_t), mTerminalSignal(0),
  mTerminalPolicy(TP_ThreadExit),

  mOwner(0), mMIR(0)
{
  // Private constructor for wrapping of existing threads.
  // Thread is put into 'Running' state, registered into the thread list
  // and assigned an internal thread-index.
  // The owner of the thread is set to 0.

  sContainerLock.Lock();
  mThreadListIt = sThreadList.insert(sThreadList.end(), this);
  mIndex        = ++sThreadCount;
  sContainerLock.Unlock();
}

GThread::GThread(const Text_t* name, GThread_foo foo, void* arg,
		 bool detached, bool detach_on_exit) :
  mRunningState (RS_Incubating),
  mIndex        (-1),
  mThreadListIt (),
  mId           (0),
  mRootTThread  (0),

  mName     (name),
  mStartFoo (foo), mStartArg (arg),
  bDetached (detached), bDetachOnExit (detach_on_exit),
  mNice     (0),
  mStackSize(sMinStackSize),

  mSigHandlerDefault(0), mSigHandlerVector(SigMAX),
  mTerminalContext(new ucontext_t), mTerminalSignal(0),
  mTerminalPolicy(Self()->GetTerminalPolicy()),

  mOwner(Owner()), mMIR(0)
{
  // Normal constructor.
  // Thread is put into 'Incubating' state, registered into the thread list
  // and assigned an internal thread-index.
  // The owner of the thread is set be the same as the owner of the calling thread.
  // Termination policy is also copied.

  static const Exc_t _eh("GThread::GThread ");

  if (!sMainThread)
    throw(_eh + "InitMain() not called.");

  sContainerLock.Lock();
  mThreadListIt = sThreadList.insert(sThreadList.end(), this);
  mIndex        = ++sThreadCount;
  sContainerLock.Unlock();
}

GThread::~GThread()
{
  // Destructor.
  // Thread is unregistered from the thread list.

  sContainerLock.Lock();
  sThreadList.erase(mThreadListIt);
  sContainerLock.Unlock();

  delete (ucontext_t*) mTerminalContext;
}

//==============================================================================

void GThread::CleanupPush(GThread_cu_foo foo, void* arg)
{
  mCleanups.push_back(Cleanup(foo, arg));
}

void GThread::CleanupPop(bool execute_p)
{
  if (mCleanups.empty()) return;

  if (execute_p)
  {
    mCleanups.back().Execute();
  }
  mCleanups.pop_back();
}

//==============================================================================

void* GThread::thread_spawner(void* arg)
{
  GThread* self = (GThread*) arg;

  pthread_setspecific(TSD_Self, self);

  if (self->mNice)
  {
    nice(self->mNice);
  }

  self->mId = pthread_self();

  sContainerLock.Lock();
  sThreadMap[self->mId] = self;
  self->mRunningState   = RS_Running;
  self->mRootTThread    = new TThread();
  self->mRootTThread->SetName(GForm("TThread gled idx=%d", self->mIndex));
  sContainerLock.Unlock();

  void* ret = 0;

  pthread_cleanup_push(thread_reaper, self);

  // getcontext causes uber crap on mac.
#ifndef __APPLE__
  if (getcontext((ucontext_t*) self->mTerminalContext))
  {
    perror("getcontext failed:");
    ret = (void*) 1;
  }
  else
  {
    if (self->mTerminalSignal)
    {
      ret = reinterpret_cast<void*>(self->mTerminalSignal);
      switch (self->mTerminalPolicy)
      {
      case TP_ThreadExit:
	break;
      case TP_GledExit:
	Gled::theOne->Exit(self->mTerminalSignal);
	break;
      case TP_SysExit:
	gSystem->Exit(self->mTerminalSignal);
	break;
      }
    }
    else
    {
      ret = (self->mStartFoo)(self->mStartArg);
    }
  }
#else
  ret = (self->mStartFoo)(self->mStartArg);
#endif

  {
    GMutexHolder _lck(sDetachCtrlLock);
    if (self->bDetachOnExit && ! self->bDetached)
    {
      self->Detach();
    }
  }

  pthread_cleanup_pop(1);

  return ret;
}

void GThread::thread_reaper(void* arg)
{
  GThread* self = (GThread*) arg;

  sContainerLock.Lock();
  self->mRunningState = RS_Terminating;
  sContainerLock.Unlock();

  while ( ! self->mCleanups.empty())
  {
    self->mCleanups.back().Execute();
    self->mCleanups.pop_back();
  }

  sContainerLock.Lock();
  self->ClearRootTThreadRepresentation();
  sThreadMap.erase(self->mId);
  self->mRunningState = RS_Finished;
  sContainerLock.Unlock();

  if (self->bDetached)
    delete self;
}

int GThread::Spawn()
{
  static const Exc_t _eh("GThread::Spawn ");

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  if (bDetached)
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  size_t stacksize;
  pthread_attr_getstacksize(&attr, &stacksize);
  if (mStackSize && (Int_t) stacksize < mStackSize)
  {
    ISmess(_eh + "increasing stack size for thread '" + mName +"' to " +
	   GForm("%dkB (was %dkB).", mStackSize/1024, (Int_t)stacksize/1024));
    pthread_attr_setstacksize(&attr, mStackSize);
  }

  sContainerLock.Lock();
  mRunningState = RS_Spawning;
  sContainerLock.Unlock();

  int ret = pthread_create(&mId, &attr, thread_spawner, this);
  if (ret)
  {
    sContainerLock.Lock();
    mRunningState = RS_ErrorSpawning;
    sContainerLock.Unlock();
    perror("GThread::Spawn");
  }

  return ret;
}

int GThread::Join(void** tret)
{
  int ret = pthread_join(mId, tret);
  if (ret) perror("GThread::Join");
  return ret;
}

int GThread::Kill(Signal signal)
{
  int ret = pthread_kill(mId, sSigMap[signal]);
  if(ret) perror("GThread::Kill");
  return ret;
}

int GThread::Cancel()
{
  int ret = pthread_cancel(mId);
  if(ret) perror("GThread::Cancel");
  return ret;
}

int GThread::Detach()
{
  GMutexHolder _lck(sDetachCtrlLock);

  int ret = pthread_detach(mId);
  if(ret) perror("GThread::Detach");
  else    bDetached = true;
  return ret;
}

bool GThread::IsDetached() const
{
  GMutexHolder _lck(sDetachCtrlLock);

  return bDetached;
}

bool GThread::ClearDetachOnExit()
{
  GMutexHolder _lck(sDetachCtrlLock);

  if ( ! bDetached)
  {
    bDetachOnExit = false;
    return true;
  }
  else
  {
    return false;
  }
}

void GThread::ClearRootTThreadRepresentation()
{
  // Wipe root's TThread representation of the calling thread.
  // This is needed as root's TSD schema expects TThread representation
  // does not exist for whatever it calls the main thread.
  // So we wipe it from the Root application thread.

  delete mRootTThread;
  mRootTThread = 0;
}


/**************************************************************************/
// Static functions; to be called from within thread.
/**************************************************************************/

GThread::CState GThread::SetCancelState(CState s)
{
  int ex_val;
  if(s == CS_Enable)
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,  &ex_val);
  else
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &ex_val);
  return ex_val == PTHREAD_CANCEL_ENABLE ? CS_Enable : CS_Disable;
}

//==============================================================================

GThread::CType GThread::SetCancelType(CType t)
{
  int ex_val;
  if(t == CT_Async)
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &ex_val);
  else
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,     &ex_val);
  return ex_val == PTHREAD_CANCEL_DEFERRED ? CT_Deferred : CT_Async;
}

//==============================================================================

int GThread::Yield()
{
  int ret = sched_yield();
  if (ret) perror("GThread::Yield");
  return ret;
}

void GThread::TestCancel()
{
  pthread_testcancel();
}

void GThread::Exit(void* ret)
{
  pthread_exit(ret);
}

//==============================================================================

GThread* GThread::Self()
{
  // Static - returns current thread.

  return (GThread*) pthread_getspecific(TSD_Self);
}

ZMirEmittingEntity* GThread::Owner()
{
  // Static - returns owner of current thread.

  return Self()->mOwner;
}

ZMIR* GThread::MIR()
{
  // Static - returns MIR processed in current thread.

  return Self()->mMIR;
}

//==============================================================================

void GThread::BlockAllSignals()
{
  // Block all signals but the unblockable ones - KILL, STOP.

  sigset_t set;
  sigfillset(&set);
  pthread_sigmask(SIG_BLOCK, &set, 0);
}

void GThread::UnblockCpuExceptionSignals(bool unblock_fpe)
{
  // Unblock CPU exception signals SIGILL, SIGSEGV, SIGBUS
  // and, if unblock_fpe is true, also SIGFPE.

  UnblockSignal(SigILL);
  UnblockSignal(SigSEGV);
  UnblockSignal(SigBUS);
  if (unblock_fpe)
    UnblockSignal(SigFPE);
}

void GThread::BlockSignal(Signal sig)
{
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, sSigMap[sig]);
  pthread_sigmask(SIG_UNBLOCK, &set, 0);
}

void GThread::UnblockSignal(Signal sig)
{
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, sSigMap[sig]);
  pthread_sigmask(SIG_UNBLOCK, &set, 0);
}

GThread_sh_foo GThread::SetDefaultSignalHandler(GThread_sh_foo foo)
{
  GThread *self = Self();
  GThread_sh_foo old = self->mSigHandlerDefault;
  self->mSigHandlerDefault = foo;
  return old;
}

GThread_sh_foo GThread::SetSignalHandler(Signal sig, GThread_sh_foo foo)
{
  GThread *self = Self();
  GThread_sh_foo old = self->mSigHandlerVector[sig];
  self->mSigHandlerVector[sig] = foo;
  return old;
}

void GThread::TheSignalHandler(GSignal* sig)
{
  static const Exc_t _eh("GThread::TheSignalHandler ");

  GThread *self = Self();

  ISdebug(1, _eh + GForm("sys_signal %d, gled_signal %d, sig_name %s in thread '%s'.",
			 sig->fSysSignal, sig->fSignal, SignalName(sig->fSignal), self->mName.Data()));

  if (sig->fSignal == SigUNDEF) return;

  if (self->mSigHandlerVector[sig->fSignal])
  {
    (self->mSigHandlerVector[sig->fSignal])(sig);
  }
  else if (self->mSigHandlerDefault != 0)
  {
    (self->mSigHandlerDefault)(sig);
  }
  else if (sig->fSignal == SigILL  ||  sig->fSignal == SigSEGV ||
	   sig->fSignal == SigBUS  ||  sig->fSignal == SigFPE)
  {
    ISerr(_eh + GForm("Fatal exception %s in thread '%s'.",
		      SignalName(sig->fSignal), self->mName.Data()));
    gSystem->StackTrace();
    self->mTerminalSignal = sig->fSignal;
    if (setcontext((ucontext_t*) self->mTerminalContext))
    {
      perror(_eh + "setcontext failed (will exit):");
      gSystem->Exit(self->mTerminalSignal);
    }
  }
}

void GThread::ToRootsSignalHandler(GSignal* sig)
{
  // Root remaps signals in an even funnier way, see TUnixSystem.cxx.

  static const int root_sig_map[kMAXSIGNALS] =
  {
    SIGBUS,  SIGSEGV, SIGSYS, SIGPIPE, SIGILL,  SIGQUIT, SIGINT,  SIGWINCH,
    SIGALRM, SIGCHLD, SIGURG, SIGFPE,  SIGTERM, SIGUSR1, SIGUSR2
  };

  for (int i = 0; i < kMAXSIGNALS; ++i)
  {
    if (root_sig_map[i] == sig->fSysSignal)
    {
      ((TUnixSystem*)gSystem)->DispatchSignals((ESignals) i);
      break;
    }
  }
}


//==============================================================================

void GThread::ListThreads()
{
  if (sMainThread == 0)
  {
    printf("Threads not initialized.");
    return;
  }

  printf("+------------------------------------------------------------------------+\n");
  sContainerLock.Lock();
  Int_t count = 0;
  for (lpGThread_i i = sThreadList.begin(); i != sThreadList.end(); ++i)
  {
    const GThread& t = **i;
    printf("| %2d | %4d | %-24s | %-14s | %-14s |\n", ++count,
           t.mIndex, t.mName.Data(), RunningStateName(t.mRunningState),
           t.get_owner() ? t.get_owner()->GetName() : "<null>");
  }
  sContainerLock.Unlock();
  printf("+------------------------------------------------------------------------+\n");
}

void GThread::ListSignalState()
{
  sigset_t set;
  sigfillset(&set);
  pthread_sigmask(0, 0, &set);

  printf("Signal block state of thread '%s':\n", Self()->mName.Data());
  for (int i = SigMIN + 1; i < SigMAX; ++i)
  {
    printf("  %6s %d", SignalName((Signal)i), sigismember(&set, sSigMap[i]));
    if (i % 8 == 0)
      printf("\n");
  }
  printf("\n");
}

//==============================================================================

void GThread::CancelAllThreads(bool join_p)
{
  if (sMainThread == 0)
  {
    printf("Threads not initialized.");
    return;
  }

  lpGThread_t clist;

  printf("+------------------------------------------------------------------------+\n");
  printf("+- Cancel All Threads ---------------------------------------------------+\n");
  printf("+------------------------------------------------------------------------+\n");
  sContainerLock.Lock();
  Int_t count = 0;
  for (lpGThread_i i = sThreadList.begin(); i != sThreadList.end(); ++i)
  {
    ++count;
    if (*i == sMainThread || (*i)->mRunningState > RS_Running)
      continue;
    GThread& t = **i;
    printf("| %2d | %4d | %-24s | %-14s | %-14s |\n", count,
           t.mIndex, t.mName.Data(), RunningStateName(t.mRunningState),
           t.get_owner() ? t.get_owner()->GetName() : "<null>");
    clist.push_back(&t);
    t.Cancel();
  }
  sContainerLock.Unlock();
  printf("+------------------------------------------------------------------------+\n");
  if (join_p)
  {
    for (lpGThread_i i = clist.begin(); i != clist.end(); ++i)
    {
      GThread& t = **i;
      void *tret;
      int   ret;
      ret = t.Join(&tret);
      printf("%d %p\n", ret, tret);
    }
    printf("+------------------------------------------------------------------------+\n");
  }
}

//==============================================================================

namespace
{
  void signal_handler_wrapper(int sid, siginfo_t* sinfo, void* sctx)
  {
    GSignal sig(sid, sinfo, sctx);
    GThread::TheSignalHandler(&sig);
  }
}

GThread* GThread::InitMain()
{
  // This will create a GThread wrapper around the calling thread.
  // To be called from ::main thread, somewhere early during the
  // system initialization.
  //
  // Blocks all signals but CPU exception ones.
  // GThread signal handler is installed for USR1, USR2 and CPU exceptions.

  static const Exc_t _eh("GThread::InitMain ");

  if (sMainThread)
  {
    throw _eh + " already called.";
  }

  pthread_key_create(&TSD_Self, 0);

  sMainThread = new GThread("main");

  pthread_setspecific(TSD_Self, sMainThread);

  sMainThread->mId = pthread_self();

  sContainerLock.Lock();
  sThreadMap[sMainThread->mId] = sMainThread;
  sContainerLock.Unlock();

  GThread::BlockAllSignals();
  GThread::UnblockCpuExceptionSignals(true);

  struct sigaction sac;
  sac.sa_sigaction = signal_handler_wrapper;
  sigemptyset(&sac.sa_mask);
  sac.sa_flags = SA_SIGINFO;
  sigaction(sSigMap[SigUSR1], &sac, 0);
  sigaction(sSigMap[SigUSR2], &sac, 0);
  sigaction(sSigMap[SigILL],  &sac, 0);
  sigaction(sSigMap[SigSEGV], &sac, 0);
  sigaction(sSigMap[SigBUS],  &sac, 0);
  sigaction(sSigMap[SigFPE],  &sac, 0);

  return sMainThread;
}

void GThread::FiniMain()
{
  static const Exc_t _eh("GThread::FiniMain ");

  if (! sMainThread)
  {
    throw _eh + "InitMain() not called.";
  }
  if (Self() != sMainThread)
  {
    throw _eh + "not called from main thread.";
  }

  sContainerLock.Lock();

  sThreadMap.erase(sMainThread->mId);

  sMainThread->mId = 0;
  sMainThread->mRunningState = RS_Finished;
  pthread_setspecific(TSD_Self, 0);

  sContainerLock.Unlock();

  delete sMainThread;
  sMainThread = 0;
}

//==============================================================================

int GThread::GetMinStackSize()
{
  return sMinStackSize;
}

void GThread::SetMinStackSize(int ss)
{
  sMinStackSize = ss;
}

//==============================================================================

const char* GThread::RunningStateName(RState state)
{
  switch (state)
  {
    case RS_Incubating:    return "Incubating";
    case RS_Spawning:      return "Spawning";
    case RS_Running:       return "Running";
    case RS_Terminating:   return "Terminating";
    case RS_Finished:      return "Finished";
    case RS_ErrorSpawning: return "ErrorSpawning";
    default:               return "<unknown>";
  }
}

const char* GThread::SignalName(Signal sig)
{
  switch (sig)
  {
    case SigHUP:  return "HUP";
    case SigINT:  return "INT";
    case SigQUIT: return "QUIT";
    case SigILL:  return "ILL";
    case SigTRAP: return "TRAP";
    case SigABRT: return "ABRT";
    case SigBUS:  return "BUS";
    case SigFPE:  return "FPE";
    case SigKILL: return "KILL";
    case SigUSR1: return "USR1";
    case SigSEGV: return "SEGV";
    case SigUSR2: return "USR2";
    case SigPIPE: return "PIPE";
    case SigALRM: return "ALRM";
    case SigTERM: return "TERM";
    case SigCHLD: return "CHLD";
    case SigCONT: return "CONT";
    case SigSTOP: return "STOP";
    case SigTSTP: return "TSTP";
    case SigTTIN: return "TTIN";
    case SigTTOU: return "TTOU";
    case SigURG:  return "URG";
    case SigXCPU: return "XCPU";
    case SigXFSZ: return "XFSZ";
    case SigVTALRM: return "VTALRM";
    case SigPROF: return "PROF";
    case SigWINCH:return "WINCH";
    case SigIO:   return "IO";
    case SigSYS:  return "SYS";
    default:      return "<unknown>";
  }
}

GThread::Signal GThread::SysToGledSignal(Int_t sys_sig)
{
  if (sys_sig <= SigMIN || sys_sig >= SigMAX) return SigUNDEF;
  for (Int_t i = SigMIN + 1; i < SigMAX; ++i)
  {
    if (sSigMap[i] == sys_sig)
      return (Signal)i;
  }
  return SigUNDEF;
}


//==============================================================================
// GSignal
//==============================================================================

GSignal::GSignal(int sid, siginfo_t* sinfo, void* sctx) :
  fSignal(GThread::SysToGledSignal(sid)),
  fSysSignal(sid), fSigInfo(sinfo), fContext(sctx)
{}

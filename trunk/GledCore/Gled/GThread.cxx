// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GThread.h"

#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

//______________________________________________________________________________
//
// POSIX thread wrapper class.
//
// !!!! Aug 2008, Fedora 7, linux-2.6.23
// It seems that priority is bluntly ignored. Tried setting it to weird
// values and not even an error is generated.
// Must investigate. Maybe needs to be enabled by the process or in kernel?
// It works ok for process (only as root, can not change ulimit -e / -r as user).

ClassImp(GThread);

GThread*                 GThread::sMainThread   = 0;
bool                     GThread::sMainInitDone = false;
int                      GThread::sThreadCount  = 0;
map<pthread_t, GThread*> GThread::sThreadMap;
list<GThread*>           GThread::sThreadList;
GMutex                   GThread::sContainerLock;

pthread_key_t GThread::TSD_Self;

/**************************************************************************/
GThread::GThread(const Text_t* name) :
  mRunningState (RS_Running),
  mIndex        (-1),
  mThreadListIt (),
  mId           (0),

  mName     (name),
  mStartFoo (0), mStartArg (0),
  mEndFoo   (0), mEndArg   (0),
  bDetached (false),
  mNice (0),

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

GThread::GThread(const Text_t* name, GThread_foo foo, void* arg, bool detached) :
  mRunningState (RS_Incubating),
  mIndex        (-1),
  mThreadListIt (),
  mId           (0),

  mName     (name),
  mStartFoo (foo), mStartArg (arg),
  mEndFoo   (0),   mEndArg   (0),
  bDetached (detached),
  mNice (0),

  mOwner(Owner()), mMIR(0)
{
  // Normal constructor.
  // Thread is put into 'Incubating' state, registered into the thread list
  // and assigned an internal thread-index.
  // The owner of the thread is set be the same as the owner of the calling thread.

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
}

/**************************************************************************/

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
  sContainerLock.Unlock();

  void* ret = 0;

  pthread_cleanup_push(thread_reaper, self);

  ret = (self->mStartFoo)(self->mStartArg);

  pthread_cleanup_pop(1);

  return ret;
}

void GThread::thread_reaper(void* arg)
{
  GThread* self = (GThread*) arg;

  sContainerLock.Lock();
  self->mRunningState = RS_Terminating;
  sContainerLock.Unlock();

  if (self->mEndFoo != 0)
  {
    (self->mEndFoo)(self->mEndArg);
  }

  sContainerLock.Lock();
  sThreadMap.erase(self->mId);
  self->mRunningState = RS_Finished;
  sContainerLock.Unlock();

  if (self->bDetached)
    delete self;
}

int GThread::Spawn()
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  if (bDetached)
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  sContainerLock.Lock();
  mRunningState = RS_Spawning;
  sContainerLock.Unlock();

  int ret =  pthread_create(&mId, &attr, thread_spawner, this);
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
  int ret = pthread_kill(mId, (Int_t)signal);
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
  int ret = pthread_detach(mId);
  if(ret) perror("GThread::Detach");
  else    bDetached = true;
  return ret;
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

/**************************************************************************/

GThread::CType GThread::SetCancelType(CType t)
{
  int ex_val;
  if(t == CT_Async)
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &ex_val);
  else
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,     &ex_val);
  return ex_val == PTHREAD_CANCEL_DEFERRED ? CT_Deferred : CT_Async;
}

/**************************************************************************/

void GThread::TestCancel()
{
  pthread_testcancel();
}

void GThread::Exit(void* ret)
{
  pthread_exit(ret);
}

/**************************************************************************/

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

/**************************************************************************/

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

/**************************************************************************/

GThread* GThread::InitMain()
{
  // This will create a GThread wrapper around the calling thread.
  // To be called from ::main thread, somewhere early during the
  // system initialization.

  static const Exc_t _eh("GThread::InitMain ");

  if (sMainThread)
  {
    throw(_eh + " already called.");
  }

  pthread_key_create(&TSD_Self, 0);

  sMainThread = new GThread("main");

  pthread_setspecific(TSD_Self, sMainThread);

  sMainThread->mId = pthread_self();

  sContainerLock.Lock();
  sThreadMap[sMainThread->mId]  = sMainThread;
  sContainerLock.Unlock();

  return sMainThread;
}

void GThread::FiniMain()
{
  static const Exc_t _eh("GThread::FiniMain ");

  if (! sMainThread)
  {
    throw(_eh + "InitMain() not called.");
  }
  if (Self() != sMainThread)
  {
    throw(_eh + "not called from main thread.");
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

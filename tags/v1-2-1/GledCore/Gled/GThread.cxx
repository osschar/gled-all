// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GThread.h"

#include <pthread.h>
#include <stdio.h>

map<pthread_t, GThread*> GThread::sIdMap;
GMutex GThread::sIDLock;

GThread::GThread(thread_f f, void* a, bool d) :
  mStartFoo(f), mArg(a), bDetached(d)
{
  mId = 0;
}

GThread::~GThread()
{
  // This should be clean up foo; Thread can die b4 object does
  // but ... as well ... object can die b4 thread does

  sIDLock.Lock(); sIdMap.erase(mId); sIDLock.Unlock();
}

/**************************************************************************/

int GThread::Spawn()
{
  pthread_attr_t threadattr;
  pthread_attr_init(&threadattr);
  if(bDetached)
    pthread_attr_setdetachstate(&threadattr, PTHREAD_CREATE_DETACHED);
  int ret;
  sIDLock.Lock(); 
  if( (ret = pthread_create(&mId, &threadattr, mStartFoo, mArg)) ) {
    perror("GThread::Spawn");
    goto end;
  }
  ISdebug(D_THRMUT, GForm("GThread::Spawn Setting id %u to %p",  mId, this));
  sIdMap[mId] = this;
 end:
  sIDLock.Unlock();
  return ret;
}

int GThread::Join(void** tret)
{

  int ret = pthread_join(mId, tret);
  if(ret) perror("GThread::Join");
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

/**************************************************************************/

void GThread::SetCancelState(CState s)
{
  if(s == CS_Enable)
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
  else
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
}

void GThread::SetCancelType(CType t)
{
  if(t == CT_Async)
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,0);
  else
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,0);
}

void GThread::TestCancel()
{
  pthread_testcancel();
}

void GThread::Exit(void* ret)
{
  pthread_exit(ret);
}

GThread* GThread::Self()
{
  //ISdebug(D_THRMUT,_s<<"Self gives " << pthread_self() <<" w/ "<< mIdMap[ pthread_self() ]));
  int foo;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &foo);
  sIDLock.Lock(); GThread* t = sIdMap[ pthread_self() ]; sIDLock.Unlock();
  pthread_setcancelstate(foo, 0);
  return t;
}

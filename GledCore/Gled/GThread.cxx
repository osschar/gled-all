// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GThread.h"

#include <pthread.h>
#include <stdio.h>

ClassImp(GThread)

map<pthread_t, GThread*> GThread::sIdMap;
GMutex GThread::sIDLock;

pthread_key_t GThread::TSD_Self;
pthread_key_t GThread::TSD_Owner;
pthread_key_t GThread::TSD_MIR;

/**************************************************************************/

GThread::GThread(GThread_foo f, void* a, bool d) :
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

void GThread::DeleteIfDetached()
{
  GThread* self = GThread::get_self();
  if(self->GetDetached()) {
    pthread_setspecific(TSD_Self, 0);
    delete self;
  }
}

/**************************************************************************/

GThread* GThread::wrap_and_register_self(ZMirEmittingEntity* owner)
{
  GThread* t = new GThread(0,0,false);
  t->mId = pthread_self();
  sIDLock.Lock();
  sIdMap[ t->mId ] = t;
  sIDLock.Unlock();
  setup_tsd(owner);
  return t;
}

/**************************************************************************/

void GThread::init_tsd()
{
  pthread_key_create(&TSD_Self, 0);
  pthread_key_create(&TSD_Owner, 0);
  pthread_key_create(&TSD_MIR, 0);
}

void GThread::setup_tsd(ZMirEmittingEntity* owner)
{
  pthread_setspecific(TSD_Self, Self());
  set_owner(owner);
  set_mir(0);
}

void GThread::cleanup_tsd()
{
  set_mir(0);
  set_owner(0);
  pthread_setspecific(TSD_Self, 0);
}

GThread* GThread::get_self()
{
  return (GThread*)pthread_getspecific(TSD_Self);
}

void GThread::set_owner(ZMirEmittingEntity* owner)
{
  pthread_setspecific(TSD_Owner, owner);
}

ZMirEmittingEntity* GThread::get_owner()
{
  return (ZMirEmittingEntity*)pthread_getspecific(TSD_Owner);
}

void GThread::set_mir(ZMIR* mir)
{
  pthread_setspecific(TSD_MIR, mir);
}

ZMIR* GThread::get_mir()
{
  return (ZMIR*)pthread_getspecific(TSD_MIR);
}

/**************************************************************************/

int GThread::Spawn()
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  if(bDetached)
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  int ret;
  sIDLock.Lock(); 
  if( (ret = pthread_create(&mId, &attr, mStartFoo, mArg)) ) {
    perror("GThread::Spawn");
    goto end;
  }
  bRunning = true;
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
  GThread* self = get_self();
  if(self) self->bRunning = false;
  pthread_exit(ret);
}

/**************************************************************************/

GThread* GThread::Self()
{
  //ISdebug(D_THRMUT,_s<<"Self gives " << pthread_self() <<" w/ "<< mIdMap[ pthread_self() ]));
  int foo;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &foo);
  sIDLock.Lock(); GThread* t = sIdMap[ pthread_self() ]; sIDLock.Unlock();
  pthread_setcancelstate(foo, 0);
  return t;
}

GThread* GThread::TSDSelf()
{
  return get_self();
}

unsigned long GThread::RawSelf()
{
  return pthread_self();
}

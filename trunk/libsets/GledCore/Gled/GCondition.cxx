// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


//______________________________________________________________________
// GCondition
//
// pthread condition variable + mutex.
// Lock/Unlock for Signal() and Broadcast() is made automatically,
// for wait operations it *must* be called from outside.

#include "GCondition.h"

#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

//______________________________________________________________________________
//
// POSIX condition-variable wrapper class.
//
// Inherits from GMutex so that is automatically associated with this
// condition. So, simply lock/unlock the condition variable object to
// ensure that no signals are missed.
//

ClassImp(GCondition);

/**************************************************************************/

GCondition::GCondition(GMutex::Init_e e) : GMutex(e)
{
  pthread_cond_init(&mCond, 0);
}

GCondition::~GCondition()
{
  pthread_cond_destroy(&mCond);
}

/**************************************************************************/

Int_t GCondition::Wait()
{
  // Performs wait ... mutex should be locked upon calling this method.

  int ret = pthread_cond_wait(&mCond, &mMut);
  return ret;
}

Int_t GCondition::TimedWait(GTime time)
{
  // Performs timedwait for interval time.
  // Mutex should be locked upon calling this method.
  // Returns 1 for time-out, 0 for other cases.

  return TimedWaitUntil(time + GTime::Now());
}

Int_t GCondition::TimedWaitUntil(GTime time)
{
  // Performs timedwait until time.
  // Mutex should be locked upon calling this method.
  // Returns 1 for time-out, 0 for other cases.

  struct timespec timeout;
  timeout.tv_sec  = time.GetSec();
  timeout.tv_nsec = time.GetNSec();

  int retcode = pthread_cond_timedwait(&mCond, &mMut, &timeout);
  if (retcode == ETIMEDOUT)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

Int_t GCondition::Signal()
{
  int ret = pthread_cond_signal(&mCond);
  return ret;
}

Int_t GCondition::Broadcast()
{
  int ret = pthread_cond_broadcast(&mCond);
  return ret;
}

Int_t GCondition::LockSignal()
{
  Lock();
  int ret = pthread_cond_signal(&mCond);
  Unlock();
  return ret;
}

Int_t GCondition::LockBroadcast()
{
  Lock();
  int ret = pthread_cond_broadcast(&mCond);
  Unlock();
  return ret;
}


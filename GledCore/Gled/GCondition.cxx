// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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

ClassImp(GCondition)

/**************************************************************************/

GCondition::GCondition(GMutex::Init_e e) : GMutex(e) {
  pthread_cond_init(&mCond, 0);
}

GCondition::~GCondition() { pthread_cond_destroy(&mCond); }

/**************************************************************************/

Int_t GCondition::Wait()
{
  // Performs wait ... mutex should be locked upon calling this method.

  int ret = pthread_cond_wait(&mCond, &mMut);
  return ret;
}

Int_t GCondition::TimedWaitMS(UInt_t wait_ms)
{
  // Performs timedwait ... mutex should be locked upon calling this method.
  // Time given in mili-seconds.
  // Returns 1 for time-out, 0 for other cases.

  struct timeval now;
  struct timespec timeout;

  gettimeofday(&now, 0);
  int mus_add = now.tv_usec + 1000*wait_ms;
  timeout.tv_sec = now.tv_sec + mus_add/1000000;
  timeout.tv_nsec = (mus_add%1000000) * 1000;
  int retcode = pthread_cond_timedwait(&mCond, &mMut, &timeout);
  if(retcode == ETIMEDOUT) {
    return 1;
  } else {
    return 0;
  }
}

Int_t GCondition::TimedWaitMuS(UInt_t wait_mus)
{
  // Performs timedwait ... mutex should be locked upon calling this method.
  // Time given in micro-seconds.
  // Returns 1 for time-out, 0 for other cases.

  struct timeval now;
  struct timespec timeout;

  gettimeofday(&now, 0);
  int mus_add = now.tv_usec + wait_mus;
  timeout.tv_sec = now.tv_sec + mus_add/1000000;
  timeout.tv_nsec = (mus_add%1000000) * 1000;
  int retcode = pthread_cond_timedwait(&mCond, &mMut, &timeout);
  if(retcode == ETIMEDOUT) {
    return 1;
  } else {
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


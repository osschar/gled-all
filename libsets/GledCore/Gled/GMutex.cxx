// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GMutex.h"
#include <Gled/GledTypes.h>
#include <Glasses/ZGlass.h>
#include <errno.h>

//______________________________________________________________________________
//
// POSIX mutex wrapper class.
//
// Note that GMutex is used as base-class for GCondition and GSElector
// but DOES NOT have any virtual functions, nor a virtual destructor.
//
// This is to save space, as Gled uses mutexes a bit too excessively.

ClassImp(GMutex);

//extern int pthread_mutexattr_settype(pthread_mutexattr_t*, int);
//#define pthread_mutexattr_settype __pthread_mutexattr_settype
// safr ... ADAPTIVE not in glibc b4 rh7 ... falling back to FAST

GMutex::GMutex(Init_e e)
{
  // This can't fail ... so says the pthread man
  pthread_mutexattr_t	attr;
  pthread_mutexattr_init(&attr);
  switch(e) {
  case fast:
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL); break;
  case recursive:
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); break;
  case error_checking:
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK); break;
  default:
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL); break;
  }
  int ret;
  if( (ret = pthread_mutex_init(&mMut, &attr)) ) {
    perror("GMutex::GMutex");
  }
}

GMutex::~GMutex()
{
  pthread_mutex_destroy(&mMut);
}

GMutex::Lock_e GMutex::Lock()
{
  int ret = pthread_mutex_lock(&mMut);
  switch(ret) {
  case 0:	return ok;
  case EDEADLK:	return deadlock;
  default:	return bad_init;
  }
}

GMutex::Lock_e GMutex::TryLock()
{
  int ret = pthread_mutex_trylock(&mMut);
  switch(ret) {
  case 0:	return ok;
  case EBUSY:	return busy;
  default:	return bad_init;
  }
}

GMutex::Lock_e GMutex::Unlock()
{
  int ret = pthread_mutex_unlock(&mMut);
  switch(ret) {
  case 0:	return ok;
  case EPERM:	return perm_fail;
  default:	return bad_init;
  }
}

/**************************************************************************/

GLensReadHolder::GLensReadHolder(ZGlass* lens) : mLens(lens)
{ mLens->ReadLock(); }

GLensReadHolder::~GLensReadHolder()
{ mLens->ReadUnlock(); }

GLensWriteHolder::GLensWriteHolder(ZGlass* lens) : mLens(lens)
{ mLens->WriteLock(); }

GLensWriteHolder::~GLensWriteHolder()
{ mLens->WriteUnlock(); }

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GSpinLock.h"
#include <Gled/GledTypes.h>
#include <errno.h>

//______________________________________________________________________________
//
// POSIX spin-lock wrapper class.
//

ClassImp(GSpinLock);

GSpinLock::GSpinLock()
{
  int ret = pthread_spin_init(&mSpinLock, PTHREAD_PROCESS_PRIVATE);
  if(ret) {
    perror("GSpinLock::GSpinLock");
  }
}

GSpinLock::~GSpinLock()
{
  int ret = pthread_spin_destroy(&mSpinLock);
  if(ret) {
    perror("GSpinLock::~GSpinLock");
  }}

GSpinLock::Lock_e GSpinLock::Lock()
{
  int ret = pthread_spin_lock(&mSpinLock);
  switch(ret) {
  case 0:	return ok;
  case EDEADLK:	return deadlock;
  default:	return bad_init;
  }
}

GSpinLock::Lock_e GSpinLock::TryLock()
{
  int ret = pthread_spin_trylock(&mSpinLock);
  switch(ret) {
  case 0:	return ok;
  case EBUSY:	return busy;
  default:	return bad_init;
  }
}

GSpinLock::Lock_e GSpinLock::Unlock()
{
  int ret = pthread_spin_unlock(&mSpinLock);
  switch(ret) {
  case 0:	return ok;
  case EPERM:	return perm_fail;
  default:	return bad_init;
  }
}

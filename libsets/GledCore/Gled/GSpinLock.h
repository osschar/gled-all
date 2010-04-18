// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GSpinLock_H
#define GledCore_GSpinLock_H

#include <Rtypes.h>
#ifndef __CINT__
#include <pthread.h>
#endif

/**************************************************************************/
// GSpinLock
/**************************************************************************/

class GSpinLock
{
protected:
#ifndef __CINT__
  pthread_spinlock_t	mSpinLock; // X{P}
#endif

public:
  enum Lock_e { ok=0, bad_init, deadlock, busy, perm_fail };

  GSpinLock();
  ~GSpinLock();

  Lock_e Lock();
  Lock_e TryLock();
  Lock_e Unlock();

#ifndef __CINT__
#include "GSpinLock.h7"
#endif
  ClassDefNV(GSpinLock, 0);
}; // endclass GSpinLock


/**************************************************************************/
// GSpinLockHolder / AntiHolder
/**************************************************************************/

class GSpinLockHolder
{
  GSpinLock& mMutex;
public:
  GSpinLockHolder(GSpinLock& m) : mMutex(m) { mMutex.Lock();   }
  virtual ~GSpinLockHolder()             { mMutex.Unlock(); }

  ClassDef(GSpinLockHolder, 0);
};

// Usability of AntiHolder limited to cases when you're sure
// the mutex is locked exactly once.

class GSpinLockAntiHolder
{
  GSpinLock& mMutex;
public:
  GSpinLockAntiHolder(GSpinLock& m) : mMutex(m) { mMutex.Unlock();   }
  virtual ~GSpinLockAntiHolder()             { mMutex.Lock(); }

  ClassDef(GSpinLockAntiHolder, 0);
};

#endif

// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GMutex_H
#define GledCore_GMutex_H

#include <Rtypes.h>
#ifndef __CINT__
#include <pthread.h>
#endif

/**************************************************************************/
// GMutex
/**************************************************************************/

class GMutex {

protected:
#ifndef __CINT__
  pthread_mutex_t	mMut;	// X{P}
  pthread_mutexattr_t	mAttr;	// X{P}
#endif

public:
  enum Init_e { fast, recursive, error_checking };
  enum Lock_e { ok=0, bad_init, deadlock, busy, perm_fail };

  GMutex(Init_e e=fast);
  virtual ~GMutex();

  Lock_e Lock();
  Lock_e TryLock();
  Lock_e Unlock();

#ifndef __CINT__
#include "GMutex.h7"
#endif
  ClassDef(GMutex, 0)
}; // endclass GMutex


/**************************************************************************/
// GMutexHolder
/**************************************************************************/

class GMutexHolder {
  GMutex& mMutex;
 public:
  GMutexHolder(GMutex& m) : mMutex(m) { mMutex.Lock();   }
  virtual ~GMutexHolder()             { mMutex.Unlock(); }

  ClassDef(GMutexHolder, 0)
};

#endif

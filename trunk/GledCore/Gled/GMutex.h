// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GMutex_H
#define GledCore_GMutex_H

#include <Rtypes.h>
#ifndef __CINT__
#include <pthread.h>
#endif

class ZGlass;

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
// GMutexHolder / AntiHolder
/**************************************************************************/

class GMutexHolder {
  GMutex& mMutex;
 public:
  GMutexHolder(GMutex& m) : mMutex(m) { mMutex.Lock();   }
  virtual ~GMutexHolder()             { mMutex.Unlock(); }

  ClassDef(GMutexHolder, 0)
};

// Usability of AntiHolder limited to cases when you're sure
// the mutex is locked exactly once.

class GMutexAntiHolder {
  GMutex& mMutex;
 public:
  GMutexAntiHolder(GMutex& m) : mMutex(m) { mMutex.Unlock();   }
  virtual ~GMutexAntiHolder()             { mMutex.Lock(); }

  ClassDef(GMutexAntiHolder, 0)
};

/**************************************************************************/
// GLensRead/WriteHolders
/**************************************************************************/

class GLensReadHolder {
  ZGlass* mLens;
 public:
  GLensReadHolder(ZGlass* lens); 
  virtual ~GLensReadHolder();

  ClassDef(GLensReadHolder, 0)
};

class GLensWriteHolder {
  ZGlass* mLens;
 public:
  GLensWriteHolder(ZGlass* lens); 
  virtual ~GLensWriteHolder();

  ClassDef(GLensWriteHolder, 0)
};

#endif

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef _GMutex_
#define _GMutex_

// Includes
#ifndef __CINT__
#include <pthread.h>
#endif

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
  ~GMutex();
#ifndef __CINT__
#include "GMutex.h7"
#endif
  Lock_e Lock();
  Lock_e TryLock();
  Lock_e Unlock();
}; // endclass GMutex

#endif

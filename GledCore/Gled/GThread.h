// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GThread_H
#define GledCore_GThread_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>
#include <map>

typedef unsigned long int pthread_t;
typedef unsigned int pthread_key_t;
typedef void*   (*GThread_foo)(void*);
typedef void (*GThread_cu_foo)(void*);

class SaturnInfo;
class ZMIR;
class ZMirEmittingEntity;

class GThread {

  friend class Gled;
  friend class Saturn;
  friend class Mountain;
  friend class ZKing;
  friend class ZQueen;

public:
  enum CState { CS_Enable, CS_Disable };
  enum CType  { CT_Async, CT_Deferred };
  enum Signal { SigHUP=1, SigINT=2, SigQUIT=3, SigILL=4, SigTRAP=5, SigABRT=6,
		SigIOT=6, SigBUS=7, SigFPE=8, SigKILL=9, SigUSR1=10, SigSEGV=11,
		SigUSR2=12, SigPIPE=13, SigALRM=14, SigTERM=15, SigSTKFLT=16,
		SigCLD=17, SigCHLD=17, SigCONT=18, SigSTOP=19, SigTSTP=20,
		SigTTIN=21, SigTTOU=22, SigURG=23, SigXCPU=24, SigXFSZ=25,
		SigVTALRM=26, SigPROF=27, SigWINCH=28, SigPOLL=29, SigIO=29,
		SigPWR=30, SigUNUSED=31 }; // from signum.h

private:
  static 	map<pthread_t, GThread*>	sIdMap;
  static	GMutex				sIDLock;

  pthread_t	 mId;		// X{g}
  GThread_foo	 mStartFoo;	// X{gs}
  void*		 mArg;		// X{gs}
  GThread_cu_foo mEndFoo;	// X{gs}
  void*		 mEndArg;	// X{gs}
  bool		 bRunning;	// X{g}
  bool		 bDetached;	// X{g}

  static GThread* wrap_and_register_self(ZMirEmittingEntity* owner);

  static void                init_tsd();
  static void                setup_tsd(ZMirEmittingEntity* owner);
  static void                cleanup_tsd();
  static GThread*            get_self();
  static void                set_owner(ZMirEmittingEntity* owner);
  static ZMirEmittingEntity* get_owner();
  static void                set_mir(ZMIR* mir);

  static pthread_key_t TSD_Self;
  static pthread_key_t TSD_Owner;
  static pthread_key_t TSD_MIR;

public:
  GThread(GThread_foo f, void* a=0, bool d=false);
  virtual ~GThread();

  static void DeleteIfDetached();

  int	Spawn();
  int	Join(void** tret=0);
  int	Kill(Signal signal=SigSTOP);
  int	Cancel();
  int   Detach();

  static CState SetCancelState(CState s);
  static CState CancelOn()  { return SetCancelState(CS_Enable);  }
  static CState CancelOff() { return SetCancelState(CS_Disable); }

  static CType  SetCancelType(CType t);

  static void   TestCancel();
  static void   Exit(void* ret=0);

  static ZMIR*  get_mir();

  static GThread* Self();
  static GThread* TSDSelf();
  static unsigned long RawSelf();

#include "GThread.h7"
  ClassDef(GThread,0)
}; // endclass GThread

#ifndef __CINT__
#define GTHREAD_CU_PUSH pthread_cleanup_push(GThread::Self()->GetEndFoo(), \
                                             GThread::Self()->GetEndArg())
#define GTHREAD_CU_POP  pthread_cleanup_pop(0)
#endif

/**************************************************************************/
// GThreadKeepAlive
/**************************************************************************/

class GThreadKeepAlive {
  GThread::CState mExCancelState;
public:
  GThreadKeepAlive()  { mExCancelState = GThread::CancelOff(); }
  ~GThreadKeepAlive() { GThread::SetCancelState(mExCancelState); }
};

#endif

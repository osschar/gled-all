// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GThread_H
#define Gled_GThread_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>
#include <map>

typedef unsigned long int pthread_t;
typedef void*   (*thread_f)(void*);
typedef void (*thread_cu_f)(void*);

class GThread {
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

  pthread_t	mId;		// X{G}
  thread_f	mStartFoo;	// X{Gs}
  void*		mArg;		// X{Gs}
  thread_cu_f	mEndFoo;	// X{Gs}
  void*		mEndArg;	// X{Gs}
  bool		bDetached;	// X{Gs}
  // !!!! hmmph ... need void* user_data !!!!

public:
  GThread(thread_f f, void* a=0, bool d=false);
  ~GThread();

  int	Spawn();
  int	Join(void** tret=0);
  int	Kill(Signal signal=SigSTOP);
  int	Cancel();
  static void SetCancelState(CState s);
  static void SetCancelType(CType t);
  static void TestCancel();
  static void Exit(void* ret=0);

  //static void CleanUpPush(thread_cu_f f, void* a);
  
  static GThread* Self();

#include "GThread.h7"
}; // endclass GThread

#ifndef __CINT__
#define GTHREAD_CU_PUSH pthread_cleanup_push(GThread::Self()->GetEndFoo(), \
                                             GThread::Self()->GetEndArg())
#define GTHREAD_CU_POP  pthread_cleanup_pop(0)
#endif

#endif

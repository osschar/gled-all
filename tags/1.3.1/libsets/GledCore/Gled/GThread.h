// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GThread_H
#define GledCore_GThread_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>
#include <map>

// This wrong ... need internal state class.
#ifdef __CINT__
typedef unsigned long int pthread_t;
typedef unsigned int pthread_key_t;
#else
#include <pthread.h>
#endif

typedef void*   (*GThread_foo)(void*);
typedef void (*GThread_cu_foo)(void*);

class SaturnInfo;
class ZMirEmittingEntity;
class ZMIR;

class GThread
{
  friend class Gled;
  friend class Saturn;
  friend class Mountain;
  friend class ZKing;
  friend class ZQueen;

public:
  enum CState { CS_Enable, CS_Disable };
  enum CType  { CT_Async, CT_Deferred };
  enum Signal { SigHUP   =  1, SigINT   =  2, SigQUIT  =  3, SigILL   =  4,
                SigTRAP  =  5, SigABRT  =  6, SigIOT   =  6, SigBUS   =  7,
                SigFPE   =  8, SigKILL  =  9, SigUSR1  = 10, SigSEGV  = 11,
		SigUSR2  = 12, SigPIPE  = 13, SigALRM  = 14, SigTERM  = 15,
                SigSTKFLT= 16, SigCLD   = 17, SigCHLD  = 17, SigCONT  = 18,
                SigSTOP  = 19, SigTSTP  = 20, SigTTIN  = 21, SigTTOU  = 22,
                SigURG   = 23, SigXCPU  = 24, SigXFSZ  = 25, SigVTALRM= 26,
                SigPROF  = 27, SigWINCH = 28, SigPOLL  = 29, SigIO    = 29,
		SigPWR   = 30, SigUNUSED= 31
  }; // from signum.h
  enum RState { RS_Incubating, RS_Spawning,
                RS_Running,
                RS_Terminating, RS_Finished,
                RS_ErrorSpawning
  };

  class CancelDisabler
  {
    CState mExCancelState;
  public:
    CancelDisabler()  { mExCancelState = CancelOff(); }
    ~CancelDisabler() { SetCancelState(mExCancelState); }
  };

private:
  class OwnerChanger
  {
    ZMirEmittingEntity* m_owner;
  public:
    OwnerChanger(ZMirEmittingEntity* o) { GThread* s = Self(); m_owner = s->get_owner(); s->set_owner(o); }
    ~OwnerChanger()                     { GThread::Self()->set_owner(m_owner); }
  };
  class MIRChanger
  {
    ZMIR* m_mir;
  public:
    MIRChanger(ZMIR* m) { GThread* s = Self(); m_mir = s->get_mir(); s->set_mir(m); }
    ~MIRChanger()       { GThread::Self()->set_mir(m_mir); }
  };


  typedef map<pthread_t, GThread*>           mPThr2GThr_t;
  typedef map<pthread_t, GThread*>::iterator mPThr2GThr_i;

  typedef list<GThread*>           lpGThread_t;
  typedef list<GThread*>::iterator lpGThread_i;

  static GThread      *sMainThread;
  static bool          sMainInitDone;
  static int           sThreadCount;
  static mPThr2GThr_t  sThreadMap;
  static lpGThread_t   sThreadList;
  static GMutex        sContainerLock;
  static int           sMinStackSize;

  // Thread state / internals
  RState         mRunningState; // X{g}
  int            mIndex;        // X{g}
  lpGThread_i    mThreadListIt;
  pthread_t	 mId;           // X{g} This will become GThreadInternalRep*

  // Parameters of thread to be spawned.
  TString        mName;         // X{Gs}
  GThread_foo	 mStartFoo;	// X{gs}
  void*		 mStartArg;	// X{gs}
  GThread_cu_foo mEndFoo;	// X{gs}
  void*		 mEndArg;	// X{gs}
  bool		 bDetached;	// X{g}
  int            mNice;         // X{gs}
  int            mStackSize;    // X{gs}

  // TSD-like members
  ZMirEmittingEntity   *mOwner;
  ZMIR                 *mMIR;

  void                set_owner(ZMirEmittingEntity* owner) { mOwner = owner; }
  ZMirEmittingEntity* get_owner() const                    { return mOwner;  }
  void                set_mir(ZMIR* mir)                   { mMIR = mir;  }
  ZMIR*               get_mir()   const                    { return mMIR; }

  static pthread_key_t TSD_Self;

  static void* thread_spawner(void* arg);
  static void  thread_reaper(void* arg);

  GThread(const Text_t* name);
public:
  GThread(const Text_t* name, GThread_foo foo, void* arg=0, bool detached=false);
  virtual ~GThread();

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

  static GThread*            Self();
  static ZMirEmittingEntity* Owner();
  static ZMIR*               MIR();

  static const char* RunningStateName(RState state);
  static void        ListThreads();

  static GThread* InitMain();
  static void     FiniMain();

  static int      GetMinStackSize();
  static void     SetMinStackSize(int ss);

#include "GThread.h7"
  ClassDef(GThread, 0);
}; // endclass GThread

#endif
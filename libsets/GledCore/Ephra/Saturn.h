// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Saturn_H
#define GledCore_Saturn_H

#include <Gled/GledTypes.h>
#include <Glasses/ZGlass.h>
#include <Stones/ZMIR.h>

class ZGod;
class ZKing; class ZFireKing;
class ZSunQueen; class ZQueen; class ZFireQueen;
class SaturnInfo; class EyeInfo; class Ray; class EyeInfoVector;
class TextMessage;

#include <Gled/GMutex.h>
#include <Gled/GSelector.h>
#include <Gled/GCondition.h>
#include <Gled/GTime.h>
class Mountain;
class ZHistoManager;
class GThread;

class TServerSocket; class TSocket;
class TMessage;


class Saturn : public An_ID_Demangler
{
  friend class Gled;
  friend class ZKing;
  friend class ZQueen; friend class ZSunQueen;
  friend class SaturnInfo;

  typedef list<SaturnInfo*>           lpSaturnInfo_t;
  typedef list<SaturnInfo*>::iterator lpSaturnInfo_i;
  typedef list<EyeInfo*>              lpEyeInfo_t;
  typedef list<EyeInfo*>::iterator    lpEyeInfo_i;

  typedef list<TSocket*>              lpSocket_t;
  typedef list<TSocket*>::iterator    lpSocket_i;

public:
  struct SocketInfo
  {
    enum OtherSide_e { OS_Moon, OS_Eye };  // What on other side ...
    OtherSide_e	fWhat;
    ZGlass*	fLensRep;

    SaturnInfo* get_moon() { return (SaturnInfo*)fLensRep; }
    EyeInfo*	get_eye()  { return (EyeInfo*)fLensRep; }

    SocketInfo(OtherSide_e w, ZGlass* g) : fWhat(w), fLensRep(g) {}
  };

#ifndef __CINT__
  typedef hash_map<TSocket*, SocketInfo>	   hSock2SocketInfo_t;
  typedef hash_map<TSocket*, SocketInfo>::iterator hSock2SocketInfo_i;
#endif

  typedef multimap<GTime, ZMIR*>           mTime2MIR_t;
  typedef multimap<GTime, ZMIR*>::iterator mTime2MIR_i;

protected:
  GMutex		mIDLock;	// X{r} ... must allow locking to eyez
  GMutex		mEyeLock;	// sending to eyes
  GMutex		mMoonLock;	// sending to moons
  GMutex		mMasterLock;	// sending to master
  GMutex		mRulingLock;	// Exec in kings & queens

  GSelector		mSelector;	// fd select wrapper for sockets

  ZGod*			mGod;		// X{g}
  ZKing*		mSunKing;	// X{g}
  ZSunQueen*		mSunQueen;	// X{g}
  ZKing*		mKing;		// X{g}
  ZFireKing*		mFireKing;	// X{g}
  ZFireQueen*		mFireQueen;	// X{g}
  SaturnInfo*		mSunInfo;	// X{g}
  SaturnInfo*		mSaturnInfo;	// X{g}
  bool			bSunAbsolute;	// X{g}

  Int_t			mQueenLoadNum;	// X{gS}
  GCondition		mQueenLoadCnd;	// X{r}

  Mountain*		mChaItOss;	// X{g}
#ifndef __CINT__
  hID2pZGlass_t		mIDHash;
  hSock2SocketInfo_t	mSock2InfoHash;
#endif
  lpSaturnInfo_t	mMoons;
  lpEyeInfo_t		mEyes;
  TServerSocket*	mServerSocket;

  // Server Thread
  GThread*		mServerThread;
  GThread*              mShutdownThread;

  Bool_t		bAllowMoons;	// X{g}


  // Detached lens threads

#ifndef __CINT__

  struct DetachedThreadInfo
  {
    ZGlass  *fLens;
    GThread *fThread;

    DetachedThreadInfo() : fLens(0), fThread(0) {}
    DetachedThreadInfo(ZGlass *l, GThread *t) : fLens(l), fThread(t) {}
  };

  typedef list<DetachedThreadInfo>           lDetachedThreadInfo_t;
  typedef list<DetachedThreadInfo>::iterator lDetachedThreadInfo_i;

  class DetachedThreadsPerLens
  {
    struct Thread
    {
      GThread               *fThread;
      lDetachedThreadInfo_i  fMainIter;

      Thread(GThread* t, lDetachedThreadInfo_i i) : fThread(t), fMainIter(i) {}
    };
    list<Thread> fThreads;
  public:
    DetachedThreadsPerLens() {}

    bool                  IsEmpty() const;
    GThread*              GetLastThread() const;
    void                  PushBack(GThread* t, lDetachedThreadInfo_i i);
    lDetachedThreadInfo_i PopBack();
    lDetachedThreadInfo_i FindAndRemove(GThread* t);
  };

  typedef hash_map<ZGlass*, DetachedThreadsPerLens>           hpZGlass2DetachedThreadPerLens_t;
  typedef hash_map<ZGlass*, DetachedThreadsPerLens>::iterator hpZGlass2DetachedThreadPerLens_i;

  lDetachedThreadInfo_t             mDetachedThreadsList;
  hpZGlass2DetachedThreadPerLens_t  mDetachedThreadsHash;
  GMutex                            mDetachedMirMutex;

public:
  void register_detached_thread  (ZGlass *lens, GThread *thread);
  void unregister_detached_thread(ZGlass *lens, GThread *thread);
protected:
  bool cancel_and_join_thread    (ZGlass* lens, GThread* thread);

#endif

  // Saturn services ... preliminary
  ZHistoManager*	pZHistoManager;

  static int start_threads(Saturn *saturn);
  int  create_server_socket();
  int  start_server();
  int  stop_server();
  int  start_shooters();
  int  stop_shooters();

  int  stop_detached_threads();

  void socket_closed(TSocket* sock);
  void wipe_moon(SaturnInfo* moon, bool notify_sunqueen_p);
  void wipe_eye(EyeInfo* eye, bool notify_sunqueen_p);

  void fix_fire_king_id(SaturnInfo* si);
  void create_kings(const char* king, const char* whore_king);
  void arrival_of_kings(TMessage* m);

  void Enlight(ZGlass* glass, ID_t) throw(Exc_t);
  void Reflect(ZGlass* glass) throw(Exc_t);
  void Freeze(ZGlass* glass) throw(Exc_t);
  void Endark(ZGlass* glass) throw(Exc_t);

  Bool_t IsMoon(SaturnInfo* si);
  void   CopyMoons(lpSaturnInfo_t& list);

  Int_t	SockSuck();	// Called constantly from ServerThread
  SaturnInfo* FindRouteTo(SaturnInfo* target);

  void AcceptWrapper(TSocket* newsocket);
  void Accept(TSocket* newsocket) throw(Exc_t);
  void finalize_moon_connection(SaturnInfo* si);
  void finalize_eye_connection(EyeInfo* ei);

  Int_t	Manage(TSocket* sock) throw(Exc_t);

public:
  Saturn();
  virtual ~Saturn();

  static TString   HandleClientSideSaturnHandshake(TSocket*& socket);
  static TMessage* HandleClientSideMeeConnection(TSocket* socket, ZMirEmittingEntity* mee);

  void	      Create(SaturnInfo* si);
  SaturnInfo* Connect(SaturnInfo* si);
  TSocket*    MakeSocketPairAndAccept(const TString& name);
  void        OpenServerSocket();
  void	      AllowMoons();
  void	      Shutdown();

  void        LockMIRShooters(bool wait_until_queue_empty=false);
  void        UnlockMIRShooters();

  virtual ZGlass* DemangleID(ID_t id);

  Int_t Freeze();
  Int_t UnFreeze();

  // Saturn services
  ZHistoManager* GetZHistoManager();

  static const Int_t s_Gled_Protocol_Version;

  /**************************************************************************/
  // MIR and MIR Result Request handling
  /**************************************************************************/

protected:

  // MIR Result Request registration and storage
  struct mir_rr_info
  {
    GCondition*  cond;
    ZMIR_RR*     mir_rr;
    mir_rr_info(GCondition* c) : cond(c), mir_rr(0) {}
  };

#ifndef __CINT__
  typedef hash_map<UInt_t, mir_rr_info>			hReqHandle2MirRRInfo_t;
  typedef hash_map<UInt_t, mir_rr_info>::iterator	hReqHandle2MirRRInfo_i;

  hReqHandle2MirRRInfo_t 	mBeamReqHandles;
#endif

  GMutex		mBeamReqHandleMutex;
  UInt_t		mLastBeamReqHandle;

  UInt_t    register_mir_result_request(GCondition* cond);
  ZMIR_RR*  query_mir_result(UInt_t req_handle);
  void	    handle_mir_result(UInt_t req_handle, ZMIR* mirp);

  GThread*		mMIRShootingThread;
  GCondition		mMIRShootingCnd;
  GMutex		mMIRShooterRoutingLock;
  list<ZMIR*>		mMIRShootingQueue;

  GThread*		mDelayedMIRShootingThread;
  GCondition		mDelayedMIRShootingCnd;
  mTime2MIR_t		mDelayedMIRShootingQueue;

  void     markup_posted_mir(ZMIR& mir, ZMirEmittingEntity* caller=0);
  void     post_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller=0);
  void     shoot_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller,
		     bool use_own_thread=false);
  void     delayed_shoot_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller,
			     const GTime& at_time);

  void     mir_shooter();
  void     delayed_mir_shooter();

  void     generick_shoot_mir_result(ZMIR& mir, const Text_t* exc, TBuffer* buf);

public:

  void     PostMIR(auto_ptr<ZMIR>& mir);
  void     PostMIR(ZMIR* mir);

  void     ShootMIR(auto_ptr<ZMIR>& mir, bool use_own_thread=false);
  void     ShootMIR(ZMIR* mir, bool use_own_thread=false);
  void     DelayedShootMIR(auto_ptr<ZMIR>& mir, const GTime& at_time);
  void     DelayedShootMIR(ZMIR* mir, const GTime& at_time);
  ZMIR_RR* ShootMIRWaitResult(auto_ptr<ZMIR>& mir, bool use_own_thread=false);
  ZMIR_RR* ShootMIRWaitResult(ZMIR* mir, bool use_own_thread=false);

  void     ShootMIRResult(TBuffer& buf);

  // Internal MIR handling

protected:
  void report_mir_pre_demangling_error(ZMIR& mir, TString error);
  void report_mir_post_demangling_error(ZMIR& mir, TString error);

  void RouteMIR(auto_ptr<ZMIR>& mir)  throw();
  void UnfoldMIR(auto_ptr<ZMIR>& mir) throw();
  void ExecMIR(ZMIR* mir, bool lockp=true);
  void ExecMIR(auto_ptr<ZMIR>& mir, bool lockp=true);
  void ExecDetachedMIR(auto_ptr<ZMIR>& mir);

  void ForwardMIR(ZMIR& mir, SaturnInfo* route);
  void BroadcastMIR(ZMIR& mir, lpSaturnInfo_t& moons);
  void BroadcastBeamMIR(ZMIR& mir, lpSaturnInfo_t& moons);


  /**************************************************************************/
  // Ray handling ... viewer notifications.
  /**************************************************************************/

 protected:
  typedef pair<Ray*, EyeInfoVector*> RayQueueEntry_t;
  typedef list<RayQueueEntry_t>      RayQueue_t;

  Bool_t                bAcceptsRays;

  GThread*              mRayEmittingThread;
  GCondition            mRayEmittingCnd;
  RayQueue_t            mRayEmittingQueue;

  void ray_emitter();

 public:

  Bool_t AcceptsRays() const { return bAcceptsRays; }

  void   Shine(auto_ptr<Ray>& ray, EyeInfoVector* eiv);

  /**************************************************************************/
  // Internal thread structures and functions
  /**************************************************************************/

private:

  // ThreadInfo structures (passed via void* to threads)

  struct new_connection_ti
  {
    Saturn*		sat;
    TSocket*		sock;
    new_connection_ti(Saturn* s, TSocket* so) : sat(s), sock(so) {}
  };

  // Thread functions

  static void* tl_SaturnFdSucker(Saturn *s);
  static void* tl_SaturnAcceptor(new_connection_ti *ss);
  static void* tl_MIR_Router(Saturn* sat);
  static void* tl_MIR_DetachedExecutor(Saturn* sat);
  static void  tl_MIR_DetachedCleanUp(Saturn* sat);

  static void* tl_MIR_Shooter(Saturn* s);
  static void* tl_Delayed_MIR_Shooter(Saturn* s);
  static void* tl_Ray_Emitter(Saturn* s);

public:
#include "Saturn.h7"
  ClassDef(Saturn, 0);
}; // endclass Saturn

/**************************************************************************/

#ifndef __CINT__

inline ZGlass* Saturn::DemangleID(ID_t id)
{
  mIDLock.Lock();
  hID2pZGlass_i i = mIDHash.find(id);
  ZGlass *ret = (i != mIDHash.end()) ? i->second : 0;
  mIDLock.Unlock();
  return ret;
}

#endif

/**************************************************************************/

#endif

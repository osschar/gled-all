// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Saturn_H
#define Gled_Saturn_H

#include <Gled/GledTypes.h>
#include <Glasses/ZGlass.h>
#include <Glasses/SaturnInfo.h>
#include <Glasses/EyeInfo.h>
#include <Stones/ZMIR.h>

class ZGod; class ZKing; class ZFireKing; class ZSunQueen; class ZQueen;

#include <Gled/GMutex.h>
#include <Gled/GSelector.h>
#include <Gled/GCondition.h>
#include <Gled/GTime.h>
#include <Net/Ray.h>
class Forest;
class Mountain;
class ZHistoManager;
class GThread;

#include <TNamed.h>
#include <TList.h>
#include <TServerSocket.h>
#include <TSocket.h>
#include <TMonitor.h>
#include <TMessage.h>

typedef list<TSocket*>			lpSocket_t;
typedef list<TSocket*>::iterator	lpSocket_i;

class Saturn : public TObject, public An_ID_Demangler
{
  friend class Gled;
  friend class ZKing;
  friend class ZQueen; friend class ZSunQueen;
  friend class SaturnInfo;

public:
  struct SocketInfo {
    enum OtherSide_e { OS_Moon, OS_Eye };  // What on other side ...
    OtherSide_e	fWhat;
    ZGlass*	fGlass;

    SaturnInfo* get_moon() { return (SaturnInfo*)fGlass; }
    EyeInfo*	get_eye()  { return (EyeInfo*)fGlass; }

    SocketInfo(OtherSide_e w, ZGlass* g) : fWhat(w), fGlass(g) {}
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

  bool			bAllowMoons;	// X{G}

  GSelector		mSelector;	// fd select wrapper for sockets

  ZGod*			mGod;		// X{G}
  ZKing*		mSunKing;	// X{G}
  ZSunQueen*		mSunQueen;	// X{G}
  ZKing*		mKing;		// X{G}
  ZFireKing*		mFireKing;	// X{G}
  ZQueen*		mFireQueen;	// X{G}
  SaturnInfo*		mSunInfo;	// X{G}
  SaturnInfo*		mSaturnInfo;	// X{G}
  bool			bSunAbsolute;	// X{G}

  Int_t			mQueenLoadNum;	// X{GS}
  GCondition		mQueenLoadCnd;	// X{r}

  Forest*		mVer;		// X{G} Will hold URL->queen/glass maps
  Mountain*		mChaItOss;	// X{G}
#ifndef __CINT__
  hID2pZGlass_t		mIDHash;
  hSock2SocketInfo_t	mSock2InfoHash;
#endif
  lpSaturnInfo_t	mMoons;
  lpEyeInfo_t		mEyes;
  TServerSocket*	mServerSocket;

  // Server Thread
  GThread*		mServerThread;

  // Saturn services ... preliminary
  ZHistoManager*	pZHistoManager;

  int  start_server();
  int  stop_server();
  int  start_shooters();
  int  stop_shooters();

  void socket_closed(TSocket* sock);
  void wipe_moon(SaturnInfo* moon, bool notify_sunqueen_p);
  void wipe_eye(EyeInfo* eye, bool notify_sunqueen_p);

  void fix_fire_king_id(SaturnInfo* si);
  void create_kings(const char* king, const char* whore_king);
  void arrival_of_kings(TMessage* m);

  void Enlight(ZGlass* glass, ID_t) throw(string);
  void Reflect(ZGlass* glass) throw(string);
  void Endark(ZGlass* glass) throw(string);

  Int_t	SockSuck();	// Called constantly from ServerThread
  SaturnInfo* FindRouteTo(SaturnInfo* target);

  void AcceptWrapper(TSocket* newsocket);
  void Accept(TSocket* newsocket) throw(string);
  void finalize_moon_connection(SaturnInfo* si);
  void finalize_eye_connection(EyeInfo* ei);

  Int_t	Manage(TSocket* sock) throw(string);

public:
  Saturn();
  virtual ~Saturn();

  static string    HandleClientSideSaturnHandshake(TSocket*& socket);
  static TMessage* HandleClientSideMeeConnection(TSocket* socket, ZMirEmittingEntity* mee);

  void	      Create(SaturnInfo* si);
  SaturnInfo* Connect(SaturnInfo* si);
  void	      AllowMoons();
  void	      Shutdown();

  virtual ZGlass* DemangleID(ID_t id);

  Int_t Freeze();
  Int_t UnFreeze();

  Bool_t IsMoon(SaturnInfo* si);
  void CopyMoons(lpSaturnInfo_t& list);

  // Saturn services
  ZHistoManager* GetZHistoManager();

  static const Int_t s_Gled_Protocol_Version;

  /**************************************************************************/
  // MIR and MIR Result Request handling
  /**************************************************************************/

protected:

  // MIR Result Request registration and storage
  struct mir_rr_info {
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
  list<ZMIR*>		mMIRShootingQueue;

  GThread*		mDelayedMIRShootingThread;
  GCondition		mDelayedMIRShootingCnd;
  mTime2MIR_t		mDelayedMIRShootingQueue;

  void     markup_posted_mir(ZMIR& mir, ZMirEmittingEntity* caller=0);
  void     post_mir(ZMIR& mir, ZMirEmittingEntity* caller=0);
  void     shoot_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller,
		     bool use_own_thread=false);
  void     delayed_shoot_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller,
			     GTime at_time);

  void     mir_shooter();
  void     delayed_mir_shooter();

  void     generick_shoot_mir_result(const Text_t* exc, TBuffer* buf);

public:

  void     PostMIR(ZMIR& mir);

  void     ShootMIR(auto_ptr<ZMIR>& mir, bool use_own_thread=false);
  void     DelayedShootMIR(auto_ptr<ZMIR>& mir, GTime at_time);
  ZMIR_RR* ShootMIRWaitResult(auto_ptr<ZMIR>& mir, bool use_own_thread=false);

  void     ShootMIRResult(TBuffer& buf);

  // Internal MIR handling

protected:
  void report_mir_pre_demangling_error(ZMIR& mir, string error);
  void report_mir_post_demangling_error(ZMIR& mir, string error);

  void RouteMIR(ZMIR& mir)  throw();
  void UnfoldMIR(ZMIR& mir) throw();
  void ExecMIR(ZMIR& mir);
  void ForwardMIR(ZMIR& mir, SaturnInfo* route);
  void BroadcastMIR(ZMIR& mir, lpSaturnInfo_t& moons);
  void BroadcastBeamMIR(ZMIR& mir, lpSaturnInfo_t& moons);


  /**************************************************************************/
  // Ray handling ... viewer notifications.
  /**************************************************************************/

 protected:

  GThread*		mRayEmittingThread;
  GCondition		mRayEmittingCnd;
  list<Ray>		mRayEmittingQueue;

  void ray_emitter();

 public:

  void	Shine(Ray& r);
  void	SingleRay(EyeInfo* eye, Ray& r);


  /**************************************************************************/
  // Internal thread structures and functions
  /**************************************************************************/

private:

  // ThreadInfo structures (passed via void* to threads)

  struct new_connection_ti {
    Saturn*		sat;
    TSocket*		sock;
    new_connection_ti(Saturn* s, TSocket* so) : sat(s), sock(so) {}
  };

  struct mir_router_ti {
    Saturn*		sat;
    ZMIR*		mir;
    bool		delete_mir;
    mir_router_ti(Saturn* s, ZMIR* m, bool d=false) :
      sat(s), mir(m), delete_mir(d) {}
  };

  // Thread functions

  static void* tl_SaturnFdSucker(Saturn *s);
  static void* tl_SaturnAcceptor(new_connection_ti *ss);
  static void* tl_MIR_Router(mir_router_ti* arg);
  static void* tl_MIR_Shooter(Saturn* s);
  static void* tl_Delayed_MIR_Shooter(Saturn* s);
  static void* tl_Ray_Emitter(Saturn* s);

public:
#include "Saturn.h7"
  ClassDef(Saturn, 0)
}; // endclass Saturn

/**************************************************************************/

#ifndef __CINT__

inline ZGlass*
Saturn::DemangleID(ID_t id) {
  mIDLock.Lock();
  hID2pZGlass_i i = mIDHash.find(id);
  ZGlass *ret = (i != mIDHash.end()) ? i->second : 0;
  mIDLock.Unlock();
  return ret;
}

#endif

/**************************************************************************/

#endif

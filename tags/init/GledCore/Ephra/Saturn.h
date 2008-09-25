// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Saturn_H
#define Gled_Saturn_H

#include <Gled/GledTypes.h>
#include <Glasses/ZGlass.h>
class ZGod; class ZKing; class ZSunQueen; class ZQueen;
#include <Glasses/SaturnInfo.h>
#include <Glasses/EyeInfo.h>

#include <Gled/GMutex.h>
#include <Gled/GSelector.h>
#include <Gled/GCondition.h>
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

class Saturn : public TObject {
  friend class ZQueen; friend class ZSunQueen;

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

protected:
  GMutex		mIDLock;	// X{r} ... must allow locking to eyez
  GMutex		mEyeLock;	// sending to eyes
  GMutex		mMoonLock;	// sending to moons
  GMutex		mMasterLock;	// sending to master
  GMutex		mServerLock;	// accept
  GMutex		mRulingLock;	// Exec in kings & queens

  GCondition		mPendingConnection; // Accept awaits instantiation of Info object
  SaturnInfo*		mPendingMoon;
  EyeInfo*		mPendingEye;
  TSocket*		mPendingSocket;
  bool			bAllowMoons;

  GSelector		mSelector;	// fd select wrapper for sockets

  ZGod*			mGod;		// X{G}
  ZKing*		mSunKing;	// X{G}
  ZSunQueen*		mSunQueen;	// X{G}
  ZKing*		mKing;		// X{G}
  ZKing*		mFireKing;	// X{G}
  ZQueen*		mFireQueen;	// X{G}
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

  // ID, Stamp ... missing ReID ReStamp for obscene values
  ID_t			mLastID;
  TimeStamp_t		mStamp;		// current timestamp X{G}
  
  // Threads
  GThread*		mServerThread;
  
  TMessage		mMsg;
  // Saturn services ... preliminary
  ZHistoManager*	pZHistoManager;
  // Quite stupid stuff
  char			xCommand[512];

  int	start_server();
  int	stop_server();

  void  socket_closed(TSocket* sock);
  void	wipe_moon(SaturnInfo* moon, bool notify_sunqueen_p);
  void	wipe_eye(EyeInfo* eye, bool notify_sunqueen_p);

  void	fix_fire_king_id(SaturnInfo* si);
  void	create_kings(const char* king, const char* whore_king);
  void	arrival_of_kings(TMessage* m);

  void  new_moon_ready(SaturnInfo* moon);
  void  new_moon_lost();
  void	new_eye_ready(EyeInfo* eye);
  void  new_eye_lost();

  void Enlight(ZGlass* glass, ID_t) throw(string);
  void Reflect(ZGlass* glass) throw(string);
  void Endark(ZGlass* glass) throw(string);

public:
  Saturn();
  virtual ~Saturn();

  void	      Create(SaturnInfo* si);
  ID_t	      QueryFFID(SaturnInfo* si);
  SaturnInfo* Connect(SaturnInfo* si);
  void	      AllowMoons();
  void	      Shutdown();

  ZGlass*	DemangleID(ID_t id);

  Int_t Freeze();
  Int_t UnFreeze();

  Int_t	SockSuck();	// Called constantly from ServerThread
  SaturnInfo* FindRouteTo(SaturnInfo* target);

  void	AcceptWrapper();
  void	Accept() throw(string);
  Int_t	Manage(TSocket* sock) throw(string);

  void	PostMIR(ZMIR& mir);
  void	RouteMIR(ZMIR& mir);
  void	UnfoldMIR(ZMIR& mir);
  void	ExecMIR(ZMIR& mir) throw(string);
  void	ForwardMIR(ZMIR& mir, SaturnInfo* route);
  void	BroadcastMIR(ZMIR& mir, lpSaturnInfo_t& moons);
  void	BroadcastBeamMIR(ZMIR& mir, lpSaturnInfo_t& moons);

  TimeStamp_t	Shine(Ray& r);
  void		SingleRay(EyeInfo* eye, Ray& r);

  Bool_t IsMoon(SaturnInfo* si);

  // Saturn services
  ZHistoManager* GetZHistoManager();

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

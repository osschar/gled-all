// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZSunQueen_H
#define Gled_ZSunQueen_H

#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>

#include <Stones/ZMirExchangeSession.h>

class ZSunQueen : public ZQueen {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZSunQueen);

  friend class Gled;
  friend class Saturn;
  friend class ZKing;

private:
  void _init();

protected:
  SaturnInfo*		mSunInfo;       // X{GS} L{}

  ZIdentity*		mSaturnGuestId; // X{GS} L{}
  ZIdentity*		mEyeGuestId;    // X{GS} L{}

  virtual ID_t incarnate_moon(SaturnInfo* parent, SaturnInfo* moon);
  virtual ID_t incarnate_eye(SaturnInfo* parent, EyeInfo* eye);


public:
  ZSunQueen(const Text_t* n="ZSunQueen", const Text_t* t=0) : ZQueen(n,t)
  { _init(); }
  ZSunQueen(ID_t span, const Text_t* n="ZSunQueen", const Text_t* t=0) :
    ZQueen(span,n,t) { _init(); }

  virtual ~ZSunQueen();

  virtual void Bootstrap();
  virtual void AdEnlightenment();
  virtual void UnfoldFrom(ZComet& comet);

  ZIdentity* GetOrImportIdentity(const char* ident);

  void AttachIdentity(ZIdentity* id); // X{E} C{1} T{SunQueen::ID_Management}
  void DetachIdentity(ZIdentity* id); // X{E} C{1} T{SunQueen::ID_Management}

  virtual void IncarnateMoon(SaturnInfo* parent); // X{E} C{1}
  virtual void IncarnateEye(SaturnInfo* parent);  // X{E} C{1}
  virtual void CremateMoon(SaturnInfo* moon);     // X{E} C{1} T{SunQueen::SaturnConnections}
  virtual void CremateEye(EyeInfo* eye);	  // X{E} C{1} T{SunQueen::EyeConnections}


  /****************************************/
  // Handling of new connections
  /****************************************/
public:
  enum ConnReqResult_e { CRR_OK=0, CRR_ReqAuth, CRR_Denied };

private:
#ifndef __CINT__

  struct NCMData { // New Connection Master
    ZMirEmittingEntity*	fNewMEE;
    SaturnInfo*		fRequestor;

    NCMData(ZMirEmittingEntity* nmee, SaturnInfo* req) :
      fNewMEE(nmee), fRequestor(req) {}
  };

  struct NCSData { // New Connection Slave
    GCondition* 	fCond;
    ConnReqResult_e	fState;
    ID_t		fMeeID;

    NCSData(GCondition* c, ConnReqResult_e s=CRR_ReqAuth) :
      fCond(c), fState(s), fMeeID(0) {}

  };

  ZMES_map<NCMData>	mNCMasterData;	//! Only needed on SunAbsolute
  ZMES_map<NCSData>	mNCSlaveData;	//!

protected:

  void handle_mee_connection(ZMirEmittingEntity* mee, TSocket *socket);

  void initiate_saturn_connection(); // X{E} T{SunQueen::SaturnConnections}
  void initiate_eye_connection();    // X{E} T{SunQueen::EyeConnections}
  void initiate_mee_connection();
  void handle_mee_authentication(UInt_t conn_id, TSocket* socket);

  void accept_mee_connection(UInt_t conn_id, ID_t mee_id); // X{E}
  void deny_mee_connection(UInt_t conn_id);                // X{E}

  ID_t incarnate_mee(UInt_t conn_id);

  // Identities

  void attach_primary_identity(ZMirEmittingEntity* mee);
  void detach_all_identities(ZMirEmittingEntity* mee);

#endif

public:
  // Authentication protocol
  static void HandleClientSideAuthentication(TSocket* socket, UInt_t conn_id,
					     TString& identity);

#include "ZSunQueen.h7"
  ClassDef(ZSunQueen, 1) // The first queen of the Sun Absolute; handles authentication, identities and MEEs
}; // endclass ZSunQueen

GlassIODef(ZSunQueen);

#endif

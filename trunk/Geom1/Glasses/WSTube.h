// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSTube_H
#define Geom1_WSTube_H

#include <Glasses/WSSeed.h>
#include <TLorentzVector.h>
#include <TRandom.h>

#include <TMatrixD.h>

class WSTube : public WSSeed
{
  MAC_RNR_FRIENDS(WSTube);

public:
  enum ConnState_e { CS_Disconnected, CS_Connecting,
		     CS_Connected,    CS_Disconnecting };

  struct Traveler {
    ZNode*  fNode;
    Float_t fPosition;
    Float_t fVelocity;
    void*   fUserData;
    Bool_t  fShown;

    Traveler() : fNode(0) {}
    Traveler(ZNode* n, Float_t p, Float_t v, void* ud=0) :
      fNode(n), fPosition(p), fVelocity(v), fUserData(0), fShown(false) {}
    virtual ~Traveler() {}

    ClassDef(Traveler, 1);
  };

private:
  void _init();

protected:
  ZLink<ZNode>	mNodeA;		// X{GS} L{}
  ZLink<ZNode>	mNodeB;		// X{GS} L{}

  Bool_t        bWeakConnect;   // X{GS} 7 Bool()
  Float_t	mDefWidth;	// X{GS} 7 Value(-range=>[  0,1000, 1,1000], join=>1)
  Float_t	mDefSpread;	// X{GS} 7 Value(-range=>[-180,180, 1,1000], join=>1)
  Float_t	mDefTension;	// X{GS} 7 Value(-range=>[-10,10,1,100])

  TLorentzVector mVecA;		// X{GSR} 7 LorentzVector()
  TLorentzVector mSgmA;		// X{GSR} 7 LorentzVector()
  TLorentzVector mVecB;		// X{GSR} 7 LorentzVector()
  TLorentzVector mSgmB;		// X{GSR} 7 LorentzVector()

  TRandom       mRnd;

  void define_direction(ZTrans& t, TVector3& dr,
			TLorentzVector& vec, TLorentzVector& sgm);

public:
  WSTube(const Text_t* n="WSTube", const Text_t* t=0) :
    WSSeed(n,t) { _init(); }
  virtual ~WSTube();

  virtual void AdEnlightenment();
  virtual void AdEndarkenment();

  // Connection and travelers
  //-------------------------
protected:
  Float_t     mDefVelocity; // X{GS} 7 Value(-range=>[0,1e3,1,1000], -join=>1)
  Float_t     mMinWaitTime; // X{GS} 7 Value(-range=>[0,1e3,1,1000])

  ConnState_e mConnectionStauts;   // X{GS} 7 PhonyEnum(-const=>1)
  Float_t     m_conn_time; //!
  Float_t     m_conn_vel;  //!
  TMatrixD*   m_conn_cof;  //!

  void connect(WSPoint*& ap, WSPoint*& bp);
  void disconnect();
  void assert_disconnect(const Exc_t& eh);
  void conn_travel(WSPoint* p, Double_t t);

public:
  void Connect();                             // X{E}  7 MButt(-join=>1)
  void Disconnect();                          // X{E}  7 MButt()
  void AnimateConnect(Float_t velocity=0);    // X{ED} 7 MCWButt(-join=>1)
  void AnimateDisconnect(Float_t velocity=0, Bool_t delay_p=false); // X{E}  7 MCWButt()

  // Travelers
  //----------
protected:


  list<Traveler>  m_traveler_list;
  ZLink<AList>    mTravelers;      // X{GS} L{} RnrBits{0,0,0,0, 0,0,0,5}
public:
  void TravelAtoB() { MakeTraveler( mDefVelocity); } // X{E} 7 MButt(-join=>1)
  void TravelBtoA() { MakeTraveler(-mDefVelocity); } // X{E} 7 MButt()
  void MakeTraveler(Float_t velocity=0, Float_t wait_time=0); // X{E} 7 MCWButt()


  // TimeMakerClient (override from WSSeed)
  virtual void TimeTick(Double_t t, Double_t dt);

#include "WSTube.h7"
  ClassDef(WSTube, 1)
}; // endclass WSTube


#endif

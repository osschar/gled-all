// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_SolarSystem_H
#define Var1_SolarSystem_H

#include <Glasses/ZNode.h>
#include <Glasses/ZVector.h>
#include <Glasses/ODECrawler.h>

#include <Stones/TimeMakerClient.h>
#include <Stones/ZColor.h>

#include <Gled/GCondition.h>

class SolarSystem : public ZNode,
		    public ODECrawlerMaster,
                    public TimeMakerClient
{
  MAC_RNR_FRIENDS(SolarSystem);

private:
  void _init();

protected:
  ZLink<ZVector>     mBalls;      // X{GS} L{A} RnrBits{0,0,0,0, 0,0,0,5}
  ZLink<ODECrawler>  mODECrawler; // X{GS} L{A}

  Double_t           mTime;       // X{G}    7 Value(-range=>[1,10000, 1,100])
  Double_t           mTimeFac;    // X{GS}   7 Value(-range=>[0,1000,  1,10])

  ZColor             mColor;      // X{PRGS} 7 ColorButt()
  Double_t           mStarMass;   // X{GS}   7 Value()
  Double_t           mBallKappa;  // X{GS}   7 Value()

  // Planetoid creation parameters
  Double_t           mPlanetMinR; // X{GS}   7 Value(-range=>[1,100, 1,100], -join=>1)
  Double_t           mPlanetMaxR; // X{GS}   7 Value(-range=>[1,100, 1,100])
  Double_t           mOrbitMinR;  // X{GS}   7 Value(-range=>[1,10000, 1,10], -join=>1)
  Double_t           mOrbitMaxR;  // X{GS}   7 Value(-range=>[1,10000, 1,10])
  Double_t           mMaxTheta;   // X{GS}   7 Value(-range=>[0,90,    1,100])
  Double_t           mMaxEcc;     // X{GS}   7 Value(-range=>[0,1,     1,1000])

  class Storage : public ODEStorageD
  {
    GCondition    mCond;
  public:

    Storage(Int_t order, Int_t capacity=128) :
      ODEStorageD(order, capacity)
    {}

    void ResetFrom(const Storage& s)
    { ResetOrder(s.Order(), 12*s.Size()/10); }
  };

  GThread        *mIntegratorThread; //!
  GMutex          mStorageLock;      //!

  map<Double_t, ODEStorageD*> mStorageMap; //!

  ODEStorageD* get_storage() { return (ODEStorageD*) mODECrawler->GetStorage(); }

public:
  SolarSystem(const Text_t* n="SolarSystem", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~SolarSystem();

  virtual void AdEnlightenment();

  // ODE virtuals
  virtual UInt_t ODEOrder();
  virtual void   ODEDerivatives(const Double_t x, const TVectorD& y, TVectorD& d);
  virtual void   ODEStart(TVectorD& v, Double_t& x1, Double_t& x2);

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

  // Propagators. Not finished.
  void StartStorageManager(); // X{ED}
  void StopStorageManager();  // X{E}

  // Visualization
  virtual void   SetTime(Double_t t); // X{E} 7 MCWButt()

  // Builders
  virtual void   MakeStar();      // X{E} 7 MCWButt()
  virtual void   MakePlanetoid(); // X{E} 7 MCWButt()

#include "SolarSystem.h7"
  ClassDef(SolarSystem, 1);
}; // endclass SolarSystem

#endif

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_CosmicBall_H
#define Var1_CosmicBall_H

#include <Glasses/Sphere.h>

class CosmicBall : public Sphere
{
  MAC_RNR_FRIENDS(CosmicBall);
  friend class SolarSystem;

private:
  void _init();

protected:
  Double_t      mM; // X{GS}   7 Value()
  HPointD       mV; // X{GSRr} 7 HPointD()

  // Hack for orbit switching.
  Double_t      mDesiredR; // X{GS} 7 Value()

  vector<HPointF> mHistory;       //!
  Int_t           mHistorySize;   //!
  Int_t           mHistoryFirst;  //!
  Int_t           mHistoryStored; //!
  GMutex          mHistoryMoo;    //!

public:
  CosmicBall(const Text_t* n="CosmicBall", const Text_t* t=0) :
    Sphere(n,t) { _init(); }

  void StorePos();
  void ClearHistory();
  void ResizeHistory(Int_t size);

#include "CosmicBall.h7"
  ClassDef(CosmicBall, 1);
}; // endclass CosmicBall


#endif

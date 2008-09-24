// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_CosmicBall_H
#define Var1_CosmicBall_H

#include <Glasses/Sphere.h>
#include <TVector3.h>

class CosmicBall : public Sphere
{
  MAC_RNR_FRIENDS(CosmicBall);
  friend class SolarSystem;

private:
  void _init();

protected:
  Double_t      mM; // X{GS}   7 Value()
  TVector3      mV; // X{GSRr} 7 Vector3()

  struct Point
  {
    Float_t x, y, z;

    Point(Float_t a=0, Float_t b=0, Float_t c=0) : x(a), y(b), z(c) {}
    Point(const Float_t*  a) : x(a[0]), y(a[1]), z(a[2]) {}
    Point(const Double_t* a) : x(a[0]), y(a[1]), z(a[2]) {}

    void Set(const Float_t*  a) { x = a[0]; y = a[1]; z = a[2]; }
    void Set(const Double_t* a) { x = a[0]; y = a[1]; z = a[2]; }

    operator const Float_t*() const { return &x; }
    operator       Float_t*()       { return &x; }
  };

  vector<Point> mHistory;       //!
  Int_t         mHistorySize;   //!
  Int_t         mHistoryFirst;  //!
  Int_t         mHistoryStored; //!
  GMutex        mHistoryMoo;    //!

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

// $Header$

#ifndef Numerica_Moonraker_H
#define Numerica_Moonraker_H

#include <Glasses/ZNode.h>
#include <Glasses/ODECrawler.h>
#include <Stones/ZColor.h>

class Moonraker : public ZNode, public ODECrawlerMaster {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(Moonraker);

private:
  Double_t	hKappa;
  Double_t	hEscapeVelocity;

  Float_t	mScale;		// X{gS} 7 Value(-range=>[0,10,1,1000], -join=>1)
  Int_t		mLOD;		// X{gS} 7 Value(-range=>[1,50,1,1])
  ZColor	mEColor;	// X{PGS} 7 ColorButt(-join=>1)
  ZColor	mMColor;	// X{PGS} 7 ColorButt()
  ZColor	mSColor;	// X{PGS} 7 ColorButt(-join=>1)
  Float_t	mRShell;	// X{gS} 7 Value(-range=>[0,1,1,100])
  ZColor	mTColor;	// X{PGS} 7 ColorButt(-join=>1)
  Float_t	mTWidth;	// X{gS}  7 Value(-range=>[0,20,1,100])

  void _init();

protected:
  ODECrawler*	mODECrawler;	// X{gS} L{}

  Float_t	mT0Moon;	// X{gS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t	mRMoon;		// X{gS} 7 Value(-range=>[0,100,1,1000])
  Float_t	mMMoon;		// X{gS} 7 Value(-range=>[0,100,1,1000], -join=>1)
  Float_t	mDMoon;		// X{gS} 7 Value(-range=>[0,1000,1,1000])

  Float_t	mT;		// X{gS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1, -width=>6)
  Float_t	mT0;		// X{gS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1, -width=>6)
  Float_t	mT1;		// X{gS} 7 Value(-range=>[-1000,1000,1,1000], -width=>6)

  Float_t	mLat;		// X{gS} 7 Value(-range=>[-90,90,1,100], -join=>1)
  Float_t	mLon;		// X{gS} 7 Value(-range=>[-180,180,1,100])
  Float_t	mTheta;		// X{gS} 7 Value(-range=>[-90,90,1,100], -join=>1)
  Float_t	mPhi;		// X{gS} 7 Value(-range=>[-90,90,1,100])
  Float_t	mV0;		// X{gS} 7 Value(-range=>[0,10,1,10000])

  ZVec3D MoonPos(Double_t t) const;

public:
  Moonraker(const Text_t* n="Moonraker", const Text_t* t=0) : ZNode(n,t)
  { _init(); }

  // virtuals
  virtual UInt_t ODEOrder() { return 6; }
  virtual void ODEDerivatives(const Double_t x, const TVectorD& y, TVectorD& d);
  virtual void ODEStart(TVectorD& v, Double_t& x1, Double_t& x2);

#include "Moonraker.h7"
  ClassDef(Moonraker, 1)
}; // endclass Moonraker

GlassIODef(Moonraker);

#endif

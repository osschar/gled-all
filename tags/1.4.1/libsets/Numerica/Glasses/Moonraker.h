// $Id$

#ifndef Numerica_Moonraker_H
#define Numerica_Moonraker_H

#include <Glasses/ZNode.h>
#include <Glasses/ODECrawler.h>
#include <Stones/ZColor.h>

class Moonraker : public ZNode, public ODECrawlerMaster
{
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
  ZLink<ODECrawler>	mODECrawler;	// X{gS} L{}

  Double_t	mT0Moon;	// X{gS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Double_t	mRMoon;		// X{gS} 7 Value(-range=>[0,100,1,1000])
  Double_t	mMMoon;		// X{gS} 7 Value(-range=>[0,100,1,1000], -join=>1)
  Double_t	mDMoon;		// X{gS} 7 Value(-range=>[0,1000,1,1000])

  Double_t	mT;		// X{gS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1, -width=>6)
  Double_t	mT0;		// X{gS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1, -width=>6)
  Double_t	mT1;		// X{gS} 7 Value(-range=>[-1000,1000,1,1000], -width=>6)

  Double_t	mLat;		// X{gS} 7 Value(-range=>[-90,90,1,100], -join=>1)
  Double_t	mLon;		// X{gS} 7 Value(-range=>[-180,180,1,100])
  Double_t	mTheta;		// X{gS} 7 Value(-range=>[-90,90,1,100], -join=>1)
  Double_t	mPhi;		// X{gS} 7 Value(-range=>[-90,90,1,100])
  Double_t	mV0;		// X{gS} 7 Value(-range=>[0,10,1,10000])

  void MoonPos(Double_t* x, Double_t t) const;
  void MoonPos(HPointD&  x, Double_t t) const;

public:
  Moonraker(const Text_t* n="Moonraker", const Text_t* t=0) : ZNode(n,t)
  { _init(); }

  // virtuals
  virtual Int_t  ODEOrder() { return 6; }
  virtual void   ODEStart(Double_t y[], Double_t& x1, Double_t& x2);
  virtual void   ODEDerivatives(Double_t x, const Double_t y[], Double_t d[]);

#include "Moonraker.h7"
  ClassDef(Moonraker, 1);
}; // endclass Moonraker


#endif

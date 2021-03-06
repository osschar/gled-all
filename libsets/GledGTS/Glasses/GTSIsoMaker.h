// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTSIsoMaker_H
#define GTS_GTSIsoMaker_H

#include <Glasses/ZGlass.h>
#include <Glasses/GTSurf.h>

class GTSIsoMakerFunctor;


class GTSIsoMaker : public ZGlass
{
  MAC_RNR_FRIENDS(GTSIsoMaker);

public:
  enum Algo_e { A_Cartesian, A_Tetra, A_TetraBounded, A_TetraBCL };

private:
  void _init();

protected:
  ZLink<GTSurf>		mTarget;  // X{GS} L{}
  ZLink<ZGlass>         mFunctor; // X{GS} L{a}

  TString		mFormula; // X{GS} 7 Textor(-width=>24)
  Algo_e		mAlgo;    // X{GS} 7 PhonyEnum(-join=>1)
  Double_t		mValue;   // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  Double_t		mXmin;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  Double_t		mXmax;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  UInt_t		mXdiv;    // X{GS} 7 Value(-range=>[2, 10000,1])

  Double_t		mYmin;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  Double_t		mYmax;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  UInt_t		mYdiv;    // X{GS} 7 Value(-range=>[2, 10000,1])

  Double_t		mZmin;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  Double_t		mZmax;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  UInt_t		mZdiv;    // X{GS} 7 Value(-range=>[2, 10000,1])

  Bool_t                bInvertCartesian; // X{GS} 7 Bool()
  Bool_t                bInvertTetra;     // X{GS} 7 Bool()

  Double_t              mFixPointEpsilon; // X{GS} 7 Value()
  Int_t                 mFixPointMaxIter; // X{GS} 7 Value()

public:
  GTSIsoMaker(const Text_t* n="GTSIsoMaker", const Text_t* t=0) : ZGlass(n,t)
  { _init(); }

  void MakeSurface();   // X{ED} 7 MButt()

  void MakeIsoDistanceHisto(const TString& canvas_name  = "IsoDelta",
			    const TString& canvas_title = "Surface iso-value delta"); // X{ED} 7 MCWButt()
  void MovePointsOntoIsoSurface(); // X{ED} 7 MButt()

  void SetXAxis(Double_t min, Double_t max, UInt_t div); // X{E}
  void SetYAxis(Double_t min, Double_t max, UInt_t div); // X{E}
  void SetZAxis(Double_t min, Double_t max, UInt_t div); // X{E}

#include "GTSIsoMaker.h7"
  ClassDef(GTSIsoMaker, 1);
}; // endclass GTSIsoMaker


#endif

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
  Float_t		mValue;   // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  Float_t		mXmin;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  Float_t		mXmax;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  UShort_t		mXdiv;    // X{GS} 7 Value(-range=>[2, 1000,1])

  Float_t		mYmin;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  Float_t		mYmax;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  UShort_t		mYdiv;    // X{GS} 7 Value(-range=>[2, 1000,1])

  Float_t		mZmin;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  Float_t		mZmax;    // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  UShort_t		mZdiv;    // X{GS} 7 Value(-range=>[2, 1000,1])

public:
  GTSIsoMaker(const Text_t* n="GTSIsoMaker", const Text_t* t=0) : ZGlass(n,t)
  { _init(); }

  void MakeSurface(); // X{ED} 7 MButt()

  void SetXAxis(Float_t min, Float_t max, UShort_t div); // X{E}
  void SetYAxis(Float_t min, Float_t max, UShort_t div); // X{E}
  void SetZAxis(Float_t min, Float_t max, UShort_t div); // X{E}

#include "GTSIsoMaker.h7"
  ClassDef(GTSIsoMaker, 1);
}; // endclass GTSIsoMaker


#endif

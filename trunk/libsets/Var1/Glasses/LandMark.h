// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_LandMark_H
#define Var1_LandMark_H

#include <Glasses/Extendio.h>
#include <Stones/ZColor.h>

class LandMark : public Extendio
{
  MAC_RNR_FRIENDS(LandMark);

public:
  enum HeightMode_e  { HM_Absolute, HM_Relative };
  enum UpDirection_e { UD_Parametric, UD_Ranged, UD_Local };

private:
  void _init();

protected:
  Float_t mF;    //  X{GE}  7 Value(-range=>[-1e5,1e5,1,1000], -join=>1)
  Float_t mG;    //  X{GE}  7 Value(-range=>[-1e5,1e5,1,1000])
  Float_t mH;    //  X{GE}  7 Value(-range=>[-1e5,1e5,1,1000], -join=>1)
  Float_t mHRel; //  X{GE}  7 Value(-range=>[-1e5,1e5,1,1000])

  HeightMode_e mHeightMode; //  X{GS}  7 PhonyEnum()

  Float_t mPhi;  //  X{GE}  7 Value(-range=>[-1e3,1e3,1,10])
  Float_t mSx;   //  X{GST} 7 Value(-range=>[0.1,100,1,10])
  Float_t mSy;   //  X{GST} 7 Value(-range=>[0.1,100,1,10])

  ZColor  mColor; // X{PRGST} 7 ColorButt()

  void retrans(ParaSurf* ps);

  void measure_height();

public:
  LandMark(const Text_t* n="LandMark", const Text_t* t=0);
  virtual ~LandMark();

  // ?? Do we need it here?
  virtual void TimeTick(Double_t t, Double_t dt) {}

  void SetF(Float_t f);
  void SetG(Float_t g);
  void SetH(Float_t h);
  void SetHRel(Float_t hr);

  void SetFGH(Float_t f, Float_t g, Float_t h);

  void SetPhi(Float_t p);

#include "LandMark.h7"
  ClassDef(LandMark, 1);
}; // endclass LandMark

#endif

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Crawler_H
#define Var1_Crawler_H

#include <Glasses/Dynamico.h>

class Crawler : public Dynamico
{
  MAC_RNR_FRIENDS(Crawler);

public:
  enum DriveMode_e { DM_Parked, DM_ConstVelocities, DM_Controllers };

private:
  void _init();

protected:
  DriveMode_e   mDriveMode; // X{GS}   7 PhonyEnum()
  Float_t       mLevH;      // X{RGS}  7 Value(-range=>[0,2, 1,100])

  Float_t       mRayOffset; //!
  HPointF       mTerrainUp; //!

  SDesireVarF   mThrottle;  // X{RGSD} 7 DesireVar(-range=>["mThrottle.GetMin()", "mThrottle.GetMax()", 1, 10])
  SDesireVarF   mWheel;     // X{RGSD} 7 DesireVar(-range=>["mWheel.GetMin()",    "mWheel.GetMax()",    1, 50])

  SMinMaxVarF   mLaserCharge; // X{RG} 7 MinMaxVar(-const=>1)

public:
  Crawler(const Text_t* n="Crawler", const Text_t* t=0);
  virtual ~Crawler();

  virtual void SetTringula(Tringula* tring);

  virtual void TimeTick(Double_t t, Double_t dt);

  virtual void ShootLaser(); // X{E}

#include "Crawler.h7"
  ClassDef(Crawler, 1);
}; // endclass Crawler

#endif

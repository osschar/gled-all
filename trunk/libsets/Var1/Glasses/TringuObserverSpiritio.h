// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuObserverSpiritio_H
#define Var1_TringuObserverSpiritio_H

#include <Glasses/Spiritio.h>

#include <Stones/SVars.h>

class ZNode;

class TringuObserverSpiritio : public Spiritio
{
  MAC_RNR_FRIENDS(TringuObserverSpiritio);

private:
  void _init();

protected:
  SDesireVarF mFwdBck;
  SDesireVarF mLftRgt;
  SDesireVarF mUpDown;

  SDesireVarF mSpinUp; // Spin about parent's up axis

  Int_t     mStampInterval;     //  X{GS} 7 Value(-range=>[0,1000])
  Int_t     mStampCount;        //!
  Double_t  mHeight;            //  Height above tringula. X{GS} 7 Value(-range=>[-1e5, 1e5, 1, 100])

public:
  TringuObserverSpiritio(const Text_t* n="TringuObserverSpiritio", const Text_t* t=0);
  virtual ~TringuObserverSpiritio();

  virtual void TimeTick(Double_t t, Double_t dt);

  virtual void Install(TSPupilInfo* pi);

#include "TringuObserverSpiritio.h7"
  ClassDef(TringuObserverSpiritio, 1);
}; // endclass TringuObserverSpiritio

#endif

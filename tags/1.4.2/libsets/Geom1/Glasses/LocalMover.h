// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_LocalMover_H
#define Geom1_LocalMover_H

#include <Glasses/Operator.h>

class ZTrans;

class LocalMover : public Operator
{
  MAC_RNR_FRIENDS(LocalMover);

private:
  void _init();

protected:
  Bool_t   bMoveOn;   // X{GS} 7 Bool(-join=>1)
  Bool_t   bMoveInPF; // X{GS} 7 Bool()
  Double_t mDx;       // X{GS} 7 Value(-join=>1)
  Double_t mDy;       // X{GS} 7 Value(-join=>1)
  Double_t mDz;       // X{GS} 7 Value()

  Bool_t   bRotOn;    // X{GS} 7 Bool(-join=>1)
  Bool_t   bRotInPF;  // X{GS} 7 Bool()
  Double_t mPhi;      // X{GS} 7 Value(-join=>1)
  Double_t mTheta;    // X{GS} 7 Value(-join=>1)
  Double_t mEta;      // X{GS} 7 Value()

  ZTrans*  mRotMatrix; //!

public:
  LocalMover(const Text_t* n="LocalMover", const Text_t* t=0) :
    Operator(n,t) { _init(); }


#include "LocalMover.h7"
  ClassDef(LocalMover, 1);
}; // endclass LocalMover


#endif

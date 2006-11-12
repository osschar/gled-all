// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Dynamico_H
#define Var1_Dynamico_H

#include <Glasses/ZGlass.h>
#include <Stones/ZTrans.h>

class TringTvor;
class TriMesh;

class Dynamico : public ZGlass
{
  friend class Tringula;
  MAC_RNR_FRIENDS(Dynamico);

private:
  void _init();

protected:
  ZTrans     mTrans;      // X{RPG} Transform from current master
  Float_t    mV;          // X{GS}  7 Value(-range=>[-100,100, 1,1000], -join=>1)
  Float_t    mW;          // X{GS}  7 Value(-range=>[  -5,  5, 1,1000])
  Float_t    mLevH;       // X{GS}  7 Value(-range=>[   0,  5, 1,1000])

  Bool_t     bRnrSelf;    // X{GS}  7 Bool()

  ZLink<TriMesh> mMesh;   // X{GS} L{}

  UInt_t     mOPCRCCache; //!

public:
  Dynamico(const Text_t* n="Dynamico", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  ZTrans& ref_trans() { return mTrans; } // Use wisely.

#include "Dynamico.h7"
  ClassDef(Dynamico, 1)
}; // endclass Dynamico


#endif

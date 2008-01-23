// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_PSMark_H
#define Var1_PSMark_H

#include <Glasses/ZNode.h>

class ParaSurf;

class PSMark : public ZNode
{
  MAC_RNR_FRIENDS(PSMark);

private:
  void _init();

protected:
  ZLink<ParaSurf>   mParaSurf;  //  X{GS} L{A}

  Float_t mF;   //  X{GE}  7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t mG;   //  X{GE}  7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t mH;   //  X{GE}  7 Value(-range=>[-1e3,1e3,1,1000])

  void retrans();

public:
  PSMark(const Text_t* n="PSMark", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  void SetF(Float_t f);
  void SetG(Float_t g);
  void SetH(Float_t h);

#include "PSMark.h7"
  ClassDef(PSMark, 1);
}; // endclass PSMark


#endif

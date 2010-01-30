// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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
  ZLink<ParaSurf>   mParaSurf;  //  X{GS} L{aA}

  Float_t mF;    //  X{GE}  7 Value(-range=>[-1e5,1e5,1,1000], -join=>1)
  Float_t mG;    //  X{GE}  7 Value(-range=>[-1e5,1e5,1,1000])
  Float_t mH;    //  X{GE}  7 Value(-range=>[-1e5,1e5,1,1000], -join=>1)

  void retrans(ParaSurf* ps);

public:
  PSMark(const Text_t* n="PSMark", const Text_t* t=0);
  PSMark(ParaSurf* ps, const Text_t* n="PSMark", const Text_t* t=0);

  void SetF(Float_t f);
  void SetG(Float_t g);
  void SetH(Float_t h);
  void SetHRel(Float_t hr);

#include "PSMark.h7"
  ClassDef(PSMark, 1);
}; // endclass PSMark


#endif

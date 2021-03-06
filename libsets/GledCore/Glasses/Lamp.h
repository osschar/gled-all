// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Lamp_H
#define Gled_Lamp_H

#include <Stones/ZColor.h>
#include <Glasses/ZNode.h>

class Lamp : public ZNode {
  MAC_RNR_FRIENDS(Lamp);
private:
  void	_init();

protected:
  ZColor	mAmbient;		// X{PGS} 7 ColorButt(-width=>5, -join=>1)
  ZColor	mDiffuse;		// X{PGS} 7 ColorButt(-width=>5)
  ZColor	mSpecular;		// X{PGS} 7 ColorButt(-width=>5, -join=>1)
  Float_t	mLampScale;		// X{GS} 7 Value(-range=>[0,100,1,10], -width=>5);
  Float_t	mSpotExp;		// X{GS} 7 Value(-range=>[0,500,1,1000], -join=>1);
  Float_t	mSpotCutOff;		// X{GS} 7 Value(-range=>[0,180,1,10]);

  Float_t	mConstAtt;		// X{GS} 7 Value(-range=>[0,100,1,1000000], -width=>8, -join=>1);
  Float_t	mLinAtt;		// X{GS} 7 Value(-range=>[0,100,1,1000000], -width=>8);
  Float_t	mQuadAtt;		// X{GS} 7 Value(-range=>[0,100,1,1000000], -width=>8);

  // How/When to draw the lamp, turn on/off the light
  Bool_t	bDrawLamp;		// X{GS} 7 Bool()
  Bool_t	bOnIfOff; 		// X{GS} 7 Bool(-join=>1)
  Bool_t	bOffIfOn;		// X{GS} 7 Bool()

public:
  Lamp(const Text_t* n="Lamp", Text_t* t=0) : ZNode(n,t) { _init(); }

#include "Lamp.h7"

  ClassDef(Lamp, 1);
}; // endclass Lamp


#endif

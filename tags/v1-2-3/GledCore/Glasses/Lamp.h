// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Lamp_H
#define Gled_Lamp_H

#include <Stones/ZColor.h>
#include <Glasses/ZNode.h>

class Lamp : public ZNode {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(Lamp);
private:
  void	_init();

protected:
  ZColor	mAmbient;		// X{PGS} 7 ColorButt()
  ZColor	mDiffuse;		// X{PGS} 7 ColorButt()
  ZColor	mSpecular;		// X{PGS} 7 ColorButt()

  Float_t	mScale;			// X{GS} 7 Value(-range=>[0,100,1,10], -join=>1);
  Float_t	mSpotExp;		// X{GS} 7 Value(-range=>[0,500,1,1000]);
  Float_t	mSpotCutOff;		// X{GS} 7 Value(-range=>[0,180,1,10]);

  Float_t	mConstAtt;		// X{GS} 7 Value(-range=>[0,100,1,1000]);
  Float_t	mLinAtt;		// X{GS} 7 Value(-range=>[0,100,1,1000]);
  Float_t	mQuadAtt;		// X{GS} 7 Value(-range=>[0,100,1,1000]);

  // How/When to draw the lamp, turn on/off the light
  bool		bDrawLamp;		// X{GS} 7 Bool()
  bool		bOnIfOff; 		// X{GS} 7 Bool(-join=>1)
  bool		bOffIfOn;		// X{GS} 7 Bool()

public:
  Lamp(const Text_t* n="Lamp", Text_t* t=0) : ZNode(n,t) { _init(); }

#include "Lamp.h7"

  ClassDef(Lamp, 1)
}; // endclass Lamp

GlassIODef(Lamp);

#endif

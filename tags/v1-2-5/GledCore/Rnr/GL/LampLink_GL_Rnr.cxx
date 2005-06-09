// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "LampLink_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/Lamp_GL_Rnr.h>

#include <FL/gl.h>

void LampLink_GL_Rnr::_init()
{
  bWarn = true;
}

/**************************************************************************/

void LampLink_GL_Rnr::AbsorbRay(Ray& ray)
{
  bWarn = true;
}

/**************************************************************************/

void LampLink_GL_Rnr::Draw(RnrDriver* rd)
{
  static const string _eh("LampLink_GL_Rnr::Draw ");

  Lamp* lamp = mLampLink->mLamp;
  if(lamp == 0) return;

  Lamp_GL_Rnr* lamp_rnr = dynamic_cast<Lamp_GL_Rnr*>(rd->GetLensRnr(lamp));
  if(lamp_rnr == 0) {
    if(bWarn) {
      printf("%s'%s' got 0 lamp-rnr for '%s'.\n", _eh.c_str(),
	     mLampLink->Identify().c_str(), lamp->Identify().c_str());;
      bWarn = false;
    }
    return;
  }

  if(lamp_rnr->GetLampID() != -1 && mLampLink->bTurnOff) {
    lamp_rnr->LampOff(rd);
  } else if(lamp_rnr->GetLampID() == -1 && mLampLink->bTurnOn) {
    ZNode* top = rd->TopPMNode();
    if(top) {
      ZTrans* tr = lamp->ToNode(top);
      if(tr) {
	lamp_rnr->LampOn(rd, tr);
	delete tr;
	return;
      }
    }
    if(bWarn) {
      printf("%s'%s' no route to '%s'.\n", _eh.c_str(),
	     mLampLink->Identify().c_str(), lamp->Identify().c_str());;
      bWarn = false;
    }
  }
}

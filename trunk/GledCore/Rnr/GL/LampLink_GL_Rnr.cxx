// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "LampLink_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/Lamp_GL_Rnr.h>

#include <FL/gl.h>

/**************************************************************************/

void LampLink_GL_Rnr::Draw(RnrDriver* rd)
{
  static const string _eh("LampLink_GL_Rnr::Draw ");

  Lamp* lamp = mLampLink->mLamp;
  Lamp_GL_Rnr* lamp_rnr = dynamic_cast<Lamp_GL_Rnr*>(rd->GetLensRnr(lamp));
  if(lamp_rnr==0) {
    cout << _eh <<"got 0 lamp rnr.\n";
    return;
  }

  if(lamp_rnr->GetLampID() != -1 && mLampLink->bTurnOff) {
    lamp_rnr->LampOff(rd);
  } else if(lamp_rnr->GetLampID() == -1 && mLampLink->bTurnOn) {
    ZNode* top = dynamic_cast<ZNode*>(rd->TopPM());
    if(top == 0) {
      cout << _eh <<"top lens is not ZNode.\n";
      return;
    }
    ZTrans* tr = lamp->ToNode(top);
    if(tr) {
      lamp_rnr->LampOn(rd, tr);
      delete tr;
    } else {
      cout << _eh <<"no route from lamp to top.\n";
    }
  }
}

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GlobalLamps_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/Lamp_GL_Rnr.h>

#include <GL/glew.h>

namespace OS = OptoStructs;

/**************************************************************************/

void GlobalLamps_GL_Rnr::_init()
{}

/**************************************************************************/

void GlobalLamps_GL_Rnr::PreDraw(RnrDriver* rd)
{
  // Turn on all list members of glass Lamp that can be
  // traced back (via Parents) to current position matrix owner.
  // In principle should store list of

  static const Exc_t _eh("GlobalLamps_GL_Rnr::PreDraw ");

  ZGlass_GL_Rnr::PreDraw(rd);

  ZNode* top = rd->TopPMNode();

  mLampsOn.clear();
  // should reuse lamps_on and wipe out those that failed
  // should ... perhaps ... also check if the light is already on and leave it alone.
  list<Lamp*> lamps; mGlobalLamps->CopyListByGlass<Lamp>(lamps);
  for(list<Lamp*>::iterator l=lamps.begin(); l!=lamps.end(); ++l) {
    // cout <<"GlobalLamps_GL_Rnr::PreDraw lamp="<< (*l)->GetName() <<endl;
    ZTrans* tr = (*l)->ToNode(top);
    if(tr) {
      Lamp_GL_Rnr* lrnr = dynamic_cast<Lamp_GL_Rnr*>(rd->GetLensRnr(*l));
      if(lrnr==0) {
	cout << _eh <<"got 0 lamp rnr.\n";
	continue;
      }
      if(lrnr->LampOn(rd, tr) == 0)
	mLampsOn.push_back(lrnr);
      delete tr;
    } else {
      cout << _eh <<"no route from lamp to top.\n";
    }
  }
}

void GlobalLamps_GL_Rnr::PostDraw(RnrDriver* rd)
{
  for(list<Lamp_GL_Rnr*>::iterator l=mLampsOn.begin(); l!=mLampsOn.end(); ++l) {
    (*l)->LampOff(rd);
  }
  ZGlass_GL_Rnr::PostDraw(rd);
}

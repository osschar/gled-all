// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Scene
//
//

#include "Scene.h"
#include "Scene.c7"
#include <Glasses/ZQueen.h>

ClassImp(Scene)

/**************************************************************************/

void Scene::_init()
{
  mGlobLamps = 0;
  mStateMods = 0;
}

/**************************************************************************/

void Scene::AdEnlightenment()
{
  ZNode::AdEnlightenment();
  if(mGlobLamps == 0) {
    GlobalLamps* l = new GlobalLamps("Global Lamps", GForm("GlobLamps of %s", GetName()));
    mQueen->CheckIn(l);
    SetGlobLamps(l);
  }
  if(mStateMods == 0) {
    ZList* l = new ZList("State Modificators", GForm("StateMods of %s", GetName()));
    mQueen->CheckIn(l);
    SetStateMods(l);
  }
}

/**************************************************************************/

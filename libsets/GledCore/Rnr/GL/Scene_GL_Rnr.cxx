// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Scene_GL_Rnr.h"
#include <GL/gl.h>

/**************************************************************************/

void Scene_GL_Rnr::_init()
{
  // From ZGlass_GL_Rnr:
  bSuppressNameLabel = true;
}

/**************************************************************************/

// In principle Scene could handle global lamps.
// Now there's a GlobalLamps glass with its own renderer.

/*
void Scene_GL_Rnr::PreDraw(RnrDriver* rd)
{

}
*/

/*
void Scene_GL_Rnr::Draw(RnrDriver* rd)
{
  printf("rendering scene %s\n", mScene->GetName());
  ZNode_GL_Rnr::Draw(rd);
}
*/

/*
void Scene_GL_Rnr::PostDraw(RnrDriver* rd)
{

}
*/

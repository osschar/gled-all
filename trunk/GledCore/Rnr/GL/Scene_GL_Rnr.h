// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Scene_GL_RNR_H
#define GledCore_Scene_GL_RNR_H

#include <Glasses/Scene.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class Scene_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  Scene*	mScene;

public:
  Scene_GL_Rnr(Scene* idol) : ZNode_GL_Rnr(idol), mScene(idol) { _init(); }

  //virtual void PreDraw(RnrDriver* rd);
  //virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

}; // endclass Scene_GL_Rnr

#endif

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Pupil_H
#define Gled_Pupil_H

// Includes
#include <Glasses/ZNode.h>
#include <Glasses/Camera.h>

#include <Eye/OptoStructs.h>
//class Eye;
class PupilInfo;
class FTW_Leaf;
//class ZNodeMixer;
//#include <Eye/GLoodge.h>
//#include <FL/ForestView.h>

#include <RnrBase/RnrDriver.h>

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

// !!!! Pupil should be A_View of its PupilInfo
// !!!! Need locator

class Pupil : public Fl_Gl_Window {
private:
  //Eye*		mEye;
  //ForestView*	mForestView;
  //GLoodge*	mGLoodge;
  PupilInfo*	mInfo;
  FTW_Leaf*	mLeaf;
  OptoStructs::ZGlassView* mRoot;

  RnrDriver*	mDriver;

  ZNode*	mBase;
  Camera*	mCamera;

  //ZNodeMixer*	pInfoView;

  int		mMouseX, mMouseY;
  bool		bJustCamera;
  bool		bFollowBase;

  char		mLabel[128];		
  void		Label();

public:
  //Pupil(PupilInfo* info, FTW_Leaf* leaf, int w=640, int h=480, const char* l="Pupil");
  Pupil(PupilInfo* info, OptoStructs::ZGlassView* zgv, int w=640, int h=480);
  virtual ~Pupil();

  void SetProjection1();
  void SetProjection2();
  // void Rebase(ZNode* newbase, bool keeppos=true);
  void XtachCamera();

  void Render();
  void Pick(bool showparents=false);

  virtual void draw();
  virtual void draw_overlay();
  virtual int  handle(int ev);

}; // enclass Pupil

typedef list<Pupil*>		lpPupil_t;
typedef list<Pupil*>::iterator	lpPupil_i;

#endif

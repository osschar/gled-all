// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Pupil_H
#define GledCore_Pupil_H

// Includes
#include <Glasses/ZNode.h>
#include <Glasses/Camera.h>

#include <Eye/OptoStructs.h>

class PupilInfo;

#include <RnrBase/RnrDriver.h>

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

// !!!! Need locator support

class Pupil : public Fl_Gl_Window, public OptoStructs::A_View
{

protected:
  bool		_firstp;

  PupilInfo*	mInfo;
  OptoStructs::ZGlassView* mRoot;  // X{G}

  RnrDriver*	mDriver;

  ZNode*	mBase;
  Camera*	mCamera;

  int		mMouseX, mMouseY;
  bool		bJustCamera;
  bool		bFollowBase;

  string	mLabel;		
  void		label_window();

  GLTextNS::TexFont* mTexFont;

  bool		bFullScreen;
  int		mFSx, mFSy, mFSw, mFSh;

  bool		bDumpImage;
  TString	mImageName;
  void		dump_image();

public:
  Pupil(OptoStructs::ZGlassImg* infoimg, OptoStructs::ZGlassView* zgv, int w=0, int h=0);
  virtual ~Pupil();

  virtual void AbsorbRay(Ray& ray);

  void SetProjection1();
  void SetProjection2();
  void SetCameraView();

  // void Rebase(ZNode* newbase, bool keeppos=true);
  void XtachCamera();
  void JumpCameraAt(ZGlass* lens);

  void FullScreen();

  void Render();
  void Pick();

  virtual void draw();
  // virtual void draw_overlay();
  virtual int  handle(int ev);

  ////////////////////////////////////////////////////////////////
  // Picking stuff
  ////////////////////////////////////////////////////////////////

  struct glass_data {
    OptoStructs::ZGlassImg*    img;
    float                      z;
    string	               name;
    OptoStructs::lpZGlassImg_t parents;

    glass_data(OptoStructs::ZGlassImg* i, float depth, const char* n) :
      img(i), z(depth), name(n) {}
  };

#include "Pupil.h7"
}; // endclass Pupil

typedef list<Pupil*>		lpPupil_t;
typedef list<Pupil*>::iterator	lpPupil_i;

#endif

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
#include "FTW_Shell.h"
class PupilInfo;
class MTW_View;

#include <RnrBase/RnrDriver.h>

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>


class Pupil : public OptoStructs::A_View,
	      public FTW_Shell_Client,
	      public Fl_Gl_Window
{

protected:
  bool		_firstp;

  PupilInfo*	mInfo;
  OptoStructs::ZGlassView* mRoot;  // X{g}

  RnrDriver*	mDriver;

  Camera*	mCamera;
  MTW_View*	mCameraView;
  Fl_Window*	mCameraViewWin;

  ZNode*	mCamBase;
  ZTrans	mCamBaseTrans;
  ZTrans	mCamAbsTrans;

  int		mMouseX, mMouseY;

  string	mLabel;		
  void		label_window();

  GLTextNS::TexFont* mTexFont;

  bool		bFullScreen;
  int		mFSx, mFSy, mFSw, mFSh;

  bool		bDumpImage;
  TString	mImageName;
  void		dump_image();

public:
  Pupil(OptoStructs::ZGlassImg* infoimg, OptoStructs::ZGlassView* zgv,
	FTW_Shell* shell, int w=0, int h=0);
  virtual ~Pupil();

  virtual void AbsorbRay(Ray& ray);

  void SetProjection1();
  void SetProjection2();
  void SetAbsRelCamera();
  void SetCameraView();

  void TurnCamTowards(ZGlass* lens, Float_t max_dist);

  void FullScreen();

  void Render();
  void Pick();

  virtual void draw();
  // virtual void draw_overlay();
  virtual int  handle(int ev);

  static void camera_stamp_cb(Camera* cam, Pupil* pup);

  float default_distance();

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

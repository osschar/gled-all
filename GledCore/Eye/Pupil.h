// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Pupil_H
#define GledCore_Pupil_H

// Includes
#include <Glasses/ZNode.h>
#include <Glasses/Camera.h>

#include <Eye/OptoStructs.h>
#include "FTW_SubShell.h"
class PupilInfo;
class MTW_View;

#include <RnrBase/RnrDriver.h>

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>


class Pupil : public FTW_SubShell,
	      public OptoStructs::A_View,
	      public Fl_Gl_Window
{
private:
  static Fl_Gl_Window* gl_ctx_holder;

protected:

  PupilInfo*	mInfo;

  RnrDriver*	mDriver;

  Camera*	mCamera;
  MTW_View*	mCameraView;

  ZNode*	mCamBase;
  ZTrans	mCamBaseTrans;
  ZTrans	mCamAbsTrans;

  int		mMouseX, mMouseY;

  string	mLabel;		
  void		label_window();

  bool		bFullScreen;
  int		mFSx, mFSy, mFSw, mFSh;

  bool		bDumpImage;
  TString	mImageName;
  void		dump_image();

public:

  static Pupil* Create_Pupil(FTW_Shell* sh, OptoStructs::ZGlassImg* img);


  Pupil(FTW_Shell* shell, OptoStructs::ZGlassImg* infoimg, int w=0, int h=0);
  virtual ~Pupil();

  virtual void AbsorbRay(Ray& ray);

  void SetProjection1();
  void SetProjection2();
  void SetAbsRelCamera();
  void SetCameraView();

  void TurnCamTowards(ZGlass* lens, Float_t max_dist);

  void FullScreen();

  void Render();
  OptoStructs::ZGlassImg* Pick(bool make_menu_p=false);

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

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Pupil_H
#define GledCore_Pupil_H

#include <Glasses/ZNode.h>
#include <Glasses/Camera.h>
class GTime;

#include <Eye/OptoStructs.h>
#include <GledView/FTW_SubShell.h>
class PupilInfo;
class MTW_ClassView;

#include <RnrBase/RnrDriver.h>

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

class PBuffer;


class Pupil : public FTW_SubShell,
	      public OptoStructs::A_View,
	      public Fl_Gl_Window
{
private:
  static Fl_Gl_Window* gl_ctx_holder;

  void _build();
  void _check_auto_redraw();

protected:

  PupilInfo*    mInfo;
  Bool_t        bAutoRedraw;

  GLRnrDriver*  mDriver;

  Camera*	mCamera;
  MTW_View* 	mCameraView;

  ZNode*	mCamBase;
  ZTrans	mCamBaseTrans;
  ZTrans	mCamAbsTrans;
  ZTrans        mProjBase;

  OptoStructs::ZGlassImg* mOverlayImg;
  OptoStructs::ZGlassImg* mEventHandlerImg;
  OptoStructs::ZGlassImg* mBelowMouseImg;

  UInt_t        mPickBuffSize;
  UInt_t*       mPickBuff;

  int		mMouseX, mMouseY;
  int           mMPX,    mMPY;    // MousePointer
  bool          bMPIn;            // MousePointer in window

  bool		bFullScreen;
  Fl_Window*	mFullScreenWin;
  int		mFSx, mFSy, mFSw, mFSh;

  bool          bShowOverlay;

  bool		bDumpImage;
  TString	mImageName;
  Int_t		mImgNTiles;
  bool          bSignalDumpFinish;
  PBuffer*      mPBuffer;
  void		dump_image(const TString& fname);

  // rendering elements
  void rnr_default_init();
  void rnr_fake_overlay(GTime& rnr_time);
  void rnr_standard(Int_t n_tiles=1, Int_t x_i=0, Int_t y_i=0);

public:

  static Pupil* Create_Pupil(FTW_Shell* sh, OptoStructs::ZGlassImg* img);


  Pupil(FTW_Shell* shell, OptoStructs::ZGlassImg* infoimg, int w=640, int h=480);
  Pupil(FTW_Shell* shell, OptoStructs::ZGlassImg* infoimg,
	int x, int y, int w, int h);
  virtual ~Pupil();

  virtual void AbsorbRay(Ray& ray);

  void SetProjection(Int_t n_tiles=1, Int_t x_i=0, Int_t y_i=0);
  void SetAbsRelCamera();
  void SetCameraView();

  void TurnCamTowards(ZGlass* lens, Float_t max_dist);

  void FullScreen(Fl_Window* fsw);

  void Render(bool rnr_self=true, bool rnr_overlay=true);

  //--------------------------------------------------------------
  // Picking stuff

  struct pick_lens_data {
    OptoStructs::ZGlassImg*    img;
    float                      z;
    TString	               name;
    OptoStructs::lpZGlassImg_t name_stack;

    pick_lens_data(OptoStructs::ZGlassImg* i, float depth, const char* n) :
      img(i), z(depth), name(n) {}
  };

  Int_t Pick(int xpick, int ypick,
	     bool rnr_self=true, bool rnr_overlay=true);

  Int_t PickTopNameStack(A_Rnr::lNSE_t& result,
			 int  xpick,    int  ypick,
			 bool rnr_self, bool rnr_overlay);

  Int_t PickLenses(list<pick_lens_data>& result,
		   bool sort_z,        bool fill_stack,
		   int  xpick,         int  ypick,
		   bool rnr_self=true, bool rnr_overlay=true);

  OptoStructs::ZGlassImg* PickTopLens(int xpick, int ypick,
				      bool rnr_self=true, bool rnr_overlay=true);

  void PickMenu(int xpick, int ypick,
		bool rnr_self=true, bool rnr_overlay=true);


  //--------------------------------------------------------------
  // Fltk interface

  virtual void label_window(const char* l=0);
  virtual void draw();
  // virtual void draw_overlay();
  virtual int  handle(int ev);

  virtual int  overlay_pick(A_Rnr::Fl_Event& e);
  virtual int  overlay_pick_and_deliver(A_Rnr::Fl_Event& e);
  virtual int  handle_overlay(int ev);

  //--------------------------------------------------------------
  // Driver redraw: combo of GLRnrDriver, draw and handle_overlay
  
  void check_driver_redraw();
  static void redraw_timeout(Pupil* pup);


  //--------------------------------------------------------------
  // Hacks
  //--------------------------------------------------------------

  float default_distance();

 protected:

  class camera_stamp_cb : public ZGlass::RayAbsorber
  {
    Pupil* pupil;
  public:
    camera_stamp_cb(Pupil* p) : pupil(p) {}
    virtual void AbsorbRay(Ray& ray);
  };

  camera_stamp_cb mCameraCB;


  // Smooth camera home
  struct smooth_camera_home_data {
    Pupil*  pupil;
    Float_t distance;
    Float_t time;
    Float_t delta_t;
  };
  void initiate_smooth_camera_home();
  static void smooth_camera_home_cb(smooth_camera_home_data* data);

#include "Pupil.h7"
}; // endclass Pupil

typedef list<Pupil*>		lpPupil_t;
typedef list<Pupil*>::iterator	lpPupil_i;

#endif
// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Pupil_H
#define GledCore_Pupil_H

#include <Glasses/ZNode.h>
#include <Glasses/Camera.h>

#include <Eye/OptoStructs.h>
#include <GledView/FTW_Shell.h>
#include <GledView/FTW_SubShell.h>

#include <RnrBase/RnrDriver.h>

class GTime;
class PupilInfo;
class MTW_ClassView;
class FBO;

#include <GL/glew.h>
#include <FL/Fl_Gl_Window.H>


class Pupil : public Fl_Gl_Window,
	      public FTW_SubShell,
	      public OptoStructs::A_View
{
private:
  static Fl_Gl_Window* gl_ctx_holder;
  static bool          glew_init_needed;

  void _build();
  void _check_auto_redraw();

protected:
  PupilInfo*    mInfo;
  Bool_t        bAutoRedraw;

  GLRnrDriver*  mDriver;

  Camera*	mCamera;
  MTW_View* 	mCameraView;

  ZNode*	mCamBase;
  ZTrans	mCamBaseTrans; // Transforamtion of camera-base in absolute space
  ZTrans	mCamAbsTrans;  // Camera transforamtion in absolute space
  ZTrans        mProjBase;
  ZTrans        mProjMatrix;

  OptoStructs::ZGlassImgHandle mOverlayImg;
  OptoStructs::ZGlassImgHandle mEventHandlerImg;

  UInt_t        mPickBuffSize;
  UInt_t*       mPickBuff;

  int		mMouseX, mMouseY;
  int           mMPX,    mMPY;    // MousePointer
  bool          bMPIn;            // MousePointer in window

  bool		bFullScreen;
  Fl_Window*	mFullScreenWin;
  int		mFSx, mFSy, mFSw, mFSh;

  bool          bShowOverlay;
  bool          bUseEventHandler;

  bool		bDumpImage;
  bool          bCopyToScreen;
  bool          bSignalDumpFinish;
  TString	mImageName;
  Int_t		mImgNTiles;
  FBO*          mFBO;

  GThread*      mCreationThread;

  Bool_t        bStereo;
  Double_t      mFAspect, mFNear, mFFar, mFTop, mFBot, mFLft, mFRgt;

  void		dump_image(const TString& fname);

  // rendering elements
  void rnr_default_init();
  void rnr_fake_overlay(GTime& rnr_time);
  void rnr_standard(bool rnr_overlay_p, Int_t n_tiles=1, Int_t x_i=0, Int_t y_i=0);
  void rnr_stereo(bool rnr_overlay_p);

public:

  static FTW_SubShell* Create_Pupil(FTW_Shell* sh, OptoStructs::ZGlassImg* img);

  Pupil(FTW_Shell* shell, OptoStructs::ZGlassImg* infoimg, int w, int h);
  Pupil(FTW_Shell* shell, OptoStructs::ZGlassImg* infoimg, int x, int y, int w, int h);
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

  struct pick_lens_data
  {
    OptoStructs::ZGlassImg *img;
    float                   z;
    TString	            name;
    A_Rnr::lNSE_t           name_stack;

    pick_lens_data(OptoStructs::ZGlassImg* i, float depth, const char* n) :
      img(i), z(depth), name(n) {}
  };

  Int_t Pick(int xpick, int ypick,
	     bool rnr_self=true, bool rnr_overlay=true);

  Int_t PickTopNameStack(A_Rnr::lNSE_t& result, float& min_z, float& max_z,
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

  virtual void setup_rnr_event(int ev, A_Rnr::Fl_Event& e);
  virtual int  overlay_pick(A_Rnr::Fl_Event& e);
  virtual int  overlay_pick_and_deliver(A_Rnr::Fl_Event& e);
  virtual int  handle_overlay(A_Rnr::Fl_Event& e);


  //--------------------------------------------------------------
  // Menu handling, direct picking

  struct pick_menu_data : public FTW_Shell::mir_call_data_img
  {
    Pupil          *pupil;
    A_Rnr::lNSE_t  &name_stack;
    A_Rnr::lNSE_i   name_stack_iterator;

    pick_menu_data(Pupil* p, A_Rnr::lNSE_t& ns, A_Rnr::lNSE_i nsi, OptoStructs::ZGlassImg* i) :
      mir_call_data_img(i, 0, 0),
      pupil(p), name_stack(ns), name_stack_iterator(nsi)
    {}
    virtual ~pick_menu_data() {}
  };

  static void cam_towards_cb(Fl_Widget* w, Pupil::pick_menu_data* ud);
  static void cam_at_cb(Fl_Widget* w, Pupil::pick_menu_data* ud);
  static void copy_to_clipboard_cb(Fl_Widget* w, Pupil::pick_menu_data* ud);
  static void deliver_event_cb(Fl_Widget* w, Pupil::pick_menu_data* ud);

  void fill_pick_menu(A_Rnr::lNSE_t& ns, A_Rnr::lNSE_i nsi,
		      OptoStructs::ZGlassImg* img, Fl_Menu_Button& menu,
		      FTW_Shell::mir_call_data_list& mcdl,
		      const TString& prefix);


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

// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Pupil_H
#define Gled_Pupil_H

// Includes
#include <Glasses/ZNode.h>
#include <Glasses/Camera.h>

#include <Eye/OptoStructs.h>

class PupilInfo;
class FTW_Leaf;

#include <RnrBase/RnrDriver.h>

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

// !!!! Pupil should be A_View of its PupilInfo
// !!!! Need locator support

class Pupil : public Fl_Gl_Window, public OptoStructs::A_View
{

protected:
  bool		_firstp;

  PupilInfo*	mInfo;
  FTW_Leaf*	mLeaf;
  OptoStructs::ZGlassView* mRoot;  // X{G}

  RnrDriver*	mDriver;

  ZNode*	mBase;
  Camera*	mCamera;

  //ZNodeMixer*	pInfoView;

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

  void FullScreen();

  void Render();
  void Pick();

  virtual void draw();
  // virtual void draw_overlay();
  virtual int  handle(int ev);

  ////////////////////////////////////////////////////////////////
  // Picking stuff
  ////////////////////////////////////////////////////////////////

  enum pick_e { p_null=0, p_open_view,
		p_user_1=0x100, p_user_2, p_user_3, p_user_4, p_user_5
  };

  struct pick_data {
    Pupil*  pupil;
    ZGlass* lens;
    pick_e  operation;
    void*   user_data;

    pick_data(Pupil* p, ZGlass* l, pick_e o=p_null, void* ud=0) :
      pupil(p), lens(l), operation(o), user_data(ud) {}
  };

protected:
  virtual void pick_callback(Fl_Widget* w, pick_data* pd);

public:
  static void fltk_pick_callback(Fl_Widget* w, pick_data* pd);

#include "Pupil.h7"
}; // endclass Pupil

typedef list<Pupil*>		lpPupil_t;
typedef list<Pupil*>::iterator	lpPupil_i;

#endif

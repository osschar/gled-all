// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Pupil.h"
#include "FBO.h"
#include <Eye/Eye.h>
#include <GledView/MTW_ClassView.h>
#include <GledView/FTW_Shell.h>
#include <GledView/FTW_Window.h>

#include <Glasses/Camera.h>
#include <Glasses/PupilInfo.h>
#include <Gled/GledNS.h>
#include <Rnr/GL/GLTextNS.h>
#include <Ephra/Saturn.h>

#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/PupilInfo_GL_Rnr.h>

#include <Gled/GThread.h>

#include <TVirtualX.h>
#include <TSystem.h>
#include <TMath.h>

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/x.H>

#include <GL/glew.h>

#include <cmath>

#ifdef __APPLE__
extern "C" GLenum glewContextInit();
#endif

namespace OS = OptoStructs;

/**************************************************************************/
// creator foo
/**************************************************************************/

Fl_Gl_Window* Pupil::gl_ctx_holder = 0;
bool          Pupil::glew_init_needed = true;

namespace {
  class pupils_gl_ctx_holder : public Fl_Gl_Window
  {
  public:
    pupils_gl_ctx_holder() : Fl_Gl_Window(1, 1, "GL context holder")
    {
      clear_border();
    }

    virtual void draw()
    {}
  };
}

FTW_SubShell* Pupil::Create_Pupil(FTW_Shell* sh, OS::ZGlassImg* img)
{
  Pupil* p = new Pupil(sh, img, 640, 480);
  return dynamic_cast<FTW_SubShell*>(p);
}

void *SubShellCreator_GledCore_Pupil = (void*)Pupil::Create_Pupil;

/**************************************************************************/
/**************************************************************************/

namespace
{
  struct tga_header
  {
    typedef unsigned char byte;

    byte  identsize;          // size of ID field that follows 18 byte header (0 usually)
    byte  colourmaptype;      // type of colour map 0=none, 1=has palette
    byte  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short colourmapstart;     // first colour map entry in palette
    short colourmaplength;    // number of colours in palette
    byte  colourmapbits;      // number of bits per palette entry 15,16,24,32

    short xstart;             // image x origin
    short ystart;             // image y origin
    short width;              // image width in pixels
    short height;             // image height in pixels
    byte  bits;               // image bits per pixel 8,16,24,32
    byte  descriptor;         // image descriptor bits (vh flip bits)

    // pixel data follows header

    tga_header(short w, short h) {
      memset(this, 0, sizeof(tga_header));
      imagetype = 2;
      width = w; height = h; bits = 24;
    }
    void dump(FILE* fp) {
      TBufferFile b(TBuffer::kWrite);
      b << identsize << colourmaptype << imagetype;
      b << host2net(colourmapstart) << host2net(colourmaplength) << colourmapbits;
      b << host2net(xstart) << host2net(ystart) << host2net(width) << host2net(height);
      b << bits << descriptor;
      fwrite(b.Buffer(), 1, b.Length(), fp);
    }
  };
}

/**************************************************************************/

void Pupil::_build()
{
  // Hack to keep the same GL context opened all the time.
  // GLEW init is done in the draw() of the context holder.

  if(gl_ctx_holder == 0) {
    Fl_Group *cur = Fl_Group::current();
    Fl_Group::current(0);
    gl_ctx_holder = new pupils_gl_ctx_holder;
    gl_ctx_holder->end();
    Fl_Group::current(cur);
    gl_ctx_holder->show();
    // gl_ctx_holder->iconize();
  }

  mInfo = dynamic_cast<PupilInfo*>(fImg->fLens);
  assert(mInfo);
  bAutoRedraw = false;
  bStereo = mInfo->bStereo;

  label_window();
  mode(FL_RGB | FL_DOUBLE | FL_DEPTH | (bStereo ? FL_STEREO : 0));
  resizable(this);
  size_range(0, 0, 4096, 4096);

  mDriver = new GLRnrDriver(fImg->fEye, "GL");

  mCamera     = new Camera;
  mCameraView = 0;

  mDriver->SetCamFixTrans(&mCamera->ref_trans());
  mDriver->SetCamBaseTrans(&mCamBaseTrans);
  mDriver->SetCamAbsTrans(&mCamAbsTrans);
  mDriver->SetProjBase(&mProjBase);
  mDriver->SetProjMatrix(&mProjMatrix);

  mCamBase = mInfo->GetCameraBase();
  if(mCamBase == 0) {
    mCamera->RotateLF(1, 2, TMath::Pi()/2);
    mCamera->MoveLF(1, -5);
    mCamera->MoveLF(3, 2);
    mCamera->RotateLF(3, 1, 30*TMath::DegToRad());
    // CamBaseTrans remains identity.
  } else {
    // Camera remains identity.
    auto_ptr<ZTrans> t( mInfo->ToPupilFrame(mCamBase) );
    if(t.get() != 0) mCamBaseTrans = *t;
  }
  mCamAbsTrans  = mCamBaseTrans;
  mCamAbsTrans *= mCamera->RefTrans();

  mOverlayImg      = fImg->fEye->DemanglePtr(mInfo->GetOverlay());
  mEventHandlerImg = fImg->fEye->DemanglePtr(mInfo->GetEventHandler());

  mPickBuffSize = mInfo->GetBuffSize();
  mPickBuff     = 0;

  bMPIn             = false;
  bFullScreen       = false;
  bShowOverlay      = true;
  bUseEventHandler  = true;
  bDumpImage        = false;
  bCopyToScreen     = false;
  bSignalDumpFinish = false;
  mFBO              = 0;

  mCreationThread   = 0;

  _check_auto_redraw();
}

void Pupil::_check_auto_redraw()
{
  if(bAutoRedraw != mInfo->bAutoRedraw) {
    if(mInfo->bAutoRedraw) {
      mShell->RegisterROARWindow(this);
      bAutoRedraw = true;
    } else {
      mShell->UnregisterROARWindow(this);
      bAutoRedraw = false;
    }
  }
}

/**************************************************************************/
/**************************************************************************/

Pupil::Pupil(FTW_Shell* shell, OS::ZGlassImg* infoimg, int w, int h) :
  Fl_Gl_Window(w,h),
  FTW_SubShell(shell, this, this),
  OS::A_View(infoimg),
  mCameraCB(this)
{
  end();
  _build();
  size(mInfo->GetWidth(), mInfo->GetHeight());
}

Pupil::Pupil(FTW_Shell* shell, OS::ZGlassImg* infoimg, int x, int y, int w, int h) :
  Fl_Gl_Window(x, y, w, h),
  FTW_SubShell(shell, this, this),
  OS::A_View(infoimg),
  mCameraCB(this)
{
  end();
  _build();
}

Pupil::~Pupil()
{
  if(bAutoRedraw) mShell->UnregisterROARWindow(this);
  delete mDriver;
  delete mCameraView;
  delete mCamera;
  delete [] mPickBuff;
  delete mFBO;
}

/**************************************************************************/

void Pupil::AbsorbRay(Ray& ray)
{
  static const Exc_t _eh("Pupil::AbsorbRay ");

  using namespace RayNS;

  if (ray.IsBasic())
    label_window();

  if (ray.fFID != PupilInfo::FID())
    return;

  if (ray.IsChangeOf(PupilInfo::FID()) && mInfo->GetBuffSize() != mPickBuffSize)
  {
    mPickBuffSize = mInfo->GetBuffSize();
    delete [] mPickBuff;
    mPickBuff     = 0;
  }

  switch(ray.fRQN)
  {
    case RayNS::RQN_change:
    {
      _check_auto_redraw();
      if (mInfo->GetBuffSize() != mPickBuffSize)
      {
        mPickBuffSize = mInfo->GetBuffSize();
        delete [] mPickBuff;
        mPickBuff     = 0;
      }
      break;
    }

    case RayNS::RQN_link_change:
    {
      if (mOverlayImg.UpdateImg(fImg->fEye->DemanglePtr(mInfo->GetOverlay())))
	bShowOverlay = true;
      mEventHandlerImg.UpdateImg(fImg->fEye->DemanglePtr(mInfo->GetEventHandler()));
      break;
    }

    case PupilInfo::PRQN_resize_window:
    {
      if(parent() == 0)
      {
        size(mInfo->GetWidth(), mInfo->GetHeight());
      }
      break;
    }

    case PupilInfo::PRQN_camera_home:
    {
      SetAbsRelCamera();
      mCamera->Home();
      redraw();
      break;
    }

    case PupilInfo::PRQN_smooth_camera_home:
    {
      initiate_smooth_camera_home();
      break;
    }

    case PupilInfo::PRQN_redraw:
    {
      TBuffer &b = ray.LockCustomBuffer();
      b  >> bSignalDumpFinish;
      ray.UnlockCustomBuffer();

      if (visible_r())
      {
        // This used to be:
        //   fImg->fEye->BreakManageLoop();
        //   redraw();
        // But then draw() never got called when window was resized to a
        // smaller(!) width/height and ScreenDumper was running in an
        // update thread (Var1, tringula.C demo).

	make_current();
	draw();
	valid(1);
	swap_buffers();
      }
      else if (bSignalDumpFinish)
      {
	mInfo->ReceiveDumpFinishedSignal();
	bSignalDumpFinish = false;
      }
      break;
    }

    case PupilInfo::PRQN_dump_image:
    {
      TBuffer &b = ray.LockCustomBuffer();
      b >> mImageName >> mImgNTiles >> bCopyToScreen >> bSignalDumpFinish;
      ray.UnlockCustomBuffer();
      bDumpImage = true;
      if (bCopyToScreen && mImgNTiles > 1)
      {
	printf("%sdump-image-ray: copy-to-screen requested but n-tiles > 1. Disabling copy-to-screen.\n",
	       _eh.Data());
	bCopyToScreen = false;
      }
      fImg->fEye->BreakManageLoop();
      redraw();
      break;
    }
  }
}


//==============================================================================
// Projection / Model-View matrix methods
//==============================================================================

void Pupil::SetProjection(Int_t n_tiles, Int_t x_i, Int_t y_i)
{
  glGetDoublev(GL_PROJECTION_MATRIX, mProjBase.Array());

  mFAspect = mInfo->GetYFac() * w()/h();
  mFNear   = TMath::Max(mInfo->GetNearClip(), 1e-5f);
  mFFar    = mInfo->GetFarClip();

  void (*set_foo)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);

  switch(mInfo->GetProjMode())
  {
    default:
    case PupilInfo::P_Perspective:
    {
      mFTop   = mFNear * TMath::Tan(0.5*TMath::DegToRad()*mInfo->GetZFov());
      set_foo = glFrustum;
      break;
    }
    case PupilInfo::P_Orthographic:
    {
      mFTop   = 0.5 * mInfo->GetZSize();
      set_foo = glOrtho;
      break;
    }
  }

  mFBot = -mFTop;
  mFRgt =  mFTop * mFAspect;
  mFLft = -mFRgt;
  if (n_tiles > 1)
  {
    double xs = 2*mFRgt/n_tiles; mFLft += x_i * xs; mFRgt = mFLft + xs;
    double ys = 2*mFTop/n_tiles; mFTop -= y_i * ys; mFBot = mFTop - ys;
  }
  set_foo(mFLft, mFRgt, mFBot, mFTop, mFNear, mFFar);

  glGetDoublev(GL_PROJECTION_MATRIX, mProjMatrix.Array());
}

void Pupil::SetAbsRelCamera()
{
  static const Exc_t _eh("Pupil::SetAbsRelCamera ");

  ZNode *cam_base, *look_at, *up_ref;
  {
    GLensReadHolder rd_lck(mInfo);

    cam_base = mInfo->GetCameraBase();
    look_at  = mInfo->GetLookAt();
    up_ref   = mInfo->GetUpReference();
  }

  if (mCamBase != cam_base)
  {
    if (mCamBase != 0)
      mCamera->SetTrans(mCamBaseTrans * mCamera->RefTrans());

    mCamBase = 0;
    mCamBaseTrans.UnitTrans();
    if (cam_base != 0)
    {
      auto_ptr<ZTrans> t( mInfo->ToPupilFrame(cam_base) );
      if (t.get() != 0)
      {
	mCamBase = cam_base;
	mCamBaseTrans = *t;
	t->Invert();
	*t *= mCamera->RefTrans();
	mCamera->SetTrans(*t);
      }
      else
      {
	cout << _eh + "CameraBase is not connected ... ignoring.\n";
      }
    }
  }

  // Begin set-up of CamAbsTrans in 'z'.
  ZTrans z;
  if (mCamBase)
  {
    auto_ptr<ZTrans> t( mInfo->ToPupilFrame(mCamBase) );
    if (t.get() != 0)
    {
      mCamBaseTrans = *t;
      z = *t;
    }
    else
    {
      mCamBaseTrans.UnitTrans();
      cout << _eh << "CameraBase is not connected ... ignoring.\n";
    }
    z *= mCamera->RefTrans();
  }
  else
  {
    z = mCamera->RefTrans();
  }

  // Construct fwd/up vectors ... consider LookAt and UpReference.
  ZPoint c_pos( z.GetBaseVec(4) );
  ZPoint x_vec( z.GetBaseVec(1) ); // Forward vector
  ZPoint z_vec( z.GetBaseVec(3) ); // Up vector
  ZPoint y_vec;                    // Deduced from x and z vecs at the end.
  bool abs_cam_changed = false;
  bool look_at_p       = false;

  if (look_at != 0)
  {
    auto_ptr<ZTrans> t( mInfo->ToPupilFrame(look_at) );
    if (t.get() != 0)
    {
      ZPoint o_pos((*t)(1,4), (*t)(2,4), (*t)(3,4));
      x_vec = (o_pos - c_pos);
      Double_t min_dist = mInfo->GetLookAtMinDist();
      if (min_dist != 0)
      {
	Double_t dist = x_vec.Mag();
	if (dist < min_dist)
	  c_pos = o_pos - min_dist/dist*x_vec;
      }
      abs_cam_changed = true;
      look_at_p       = true;
    }
    else
    {
      cout << _eh << "LookAt is not connected ... ignoring.\n";
    }
  }

  if (up_ref != 0)
  {
    auto_ptr<ZTrans> t( mInfo->ToPupilFrame(up_ref) );
    if (t.get() != 0)
    {
      z_vec = t->GetBaseVec( mInfo->GetUpRefAxis() );
      abs_cam_changed = true;
    }
    else
    {
      cout << _eh << "UpReference not connected ... ignoring.\n";
    }
  }

  // Ortonormalize the vectors.
  if (abs_cam_changed)
  {
    if (x_vec.Mag2() == 0)
      x_vec = mCamAbsTrans.GetBaseVec(1);

    x_vec.Normalize();
    z_vec.Normalize();

    Double_t xz_dp  = z_vec.Dot(x_vec);
    Double_t max_dp = TMath::Cos(TMath::DegToRad()*mInfo->GetUpRefMinAngle());
    if (TMath::Abs(xz_dp) > max_dp)
    {
      if (mInfo->GetUpRefLockDir() && look_at_p == false)
      {
	Double_t sgn_mdp = TMath::Sign(max_dp, xz_dp);
	ZPoint   ortog = (x_vec - xz_dp*z_vec);
	ortog.Normalize();
	x_vec  = sgn_mdp*z_vec + TMath::Sqrt(1 - max_dp*max_dp)*ortog;
	x_vec.Normalize();
	xz_dp  = z_vec.Dot(x_vec);
	z_vec -= xz_dp*x_vec;
	z_vec.Normalize();
      }
      else
      {
	z_vec  = mCamAbsTrans.GetBaseVec(3);
	xz_dp  = z_vec.Dot(x_vec);
	z_vec -= xz_dp*x_vec;
	z_vec.Normalize();
      }
    }
    else
    {
      z_vec -= xz_dp*x_vec;
      z_vec.Normalize();
    }
  }
  y_vec.Cross(z_vec, x_vec);

  // Construct absolute CamAbsTrans,
  mCamAbsTrans.UnitTrans();
  mCamAbsTrans.SetBaseVec(1, x_vec);
  mCamAbsTrans.SetBaseVec(2, y_vec);
  mCamAbsTrans.SetBaseVec(3, z_vec);
  mCamAbsTrans.SetBaseVec(4, c_pos);

  // Multiply-out the CamBaseTrans to get true camera for local controls.
  if (mCamBase != 0)
  {
    ZTrans t(mCamBaseTrans);
    t.Invert();
    t *= mCamAbsTrans;
    mCamera->SetTrans(t);
  }
  else
  {
    mCamera->SetTrans(mCamAbsTrans);
  }
}

void Pupil::SetCameraView()
{
  const Double_t *Pos = mCamAbsTrans.ArrT();
  const Double_t *Fwd = mCamAbsTrans.ArrX();
  const Double_t *Up  = mCamAbsTrans.ArrZ();

  gluLookAt(Pos[0],        Pos[1],        Pos[2],
	    Pos[0]+Fwd[0], Pos[1]+Fwd[1], Pos[2]+Fwd[2],
	    Up [0],        Up [1],        Up [2]);
}

/**************************************************************************/

void Pupil::TurnCamTowards(ZGlass* lens, Float_t max_dist)
{
  // Warps camera towards lens.

  ZNode* node = dynamic_cast<ZNode*>(lens);
  if(node == 0) return;
  auto_ptr<ZTrans> t( mInfo->ToCameraFrame(node) );
  if(t.get() == 0) return;

  ZPoint x = t->GetPos() - mCamera->RefTrans().GetPos();
  Double_t dist = x.Normalize();

  ZPoint y = mCamera->RefTrans().GetBaseVec(2);
  y -= (y.Dot(x))*x;
  y.Normalize();

  Float_t to_move = 0;
  if(dist > max_dist) to_move = dist - max_dist;

  // Now reuse t to hold new camera transformation.
  t->SetBaseVec(1, x);
  t->SetBaseVec(2, y);
  t->SetBaseVec(3, x.Cross(y));
  t->SetPos(mCamera->RefTrans());
  t->MoveLF(1, to_move);
  mCamera->SetTrans(*t);

  // cout << "CameraTrans:\n" << *t;

  redraw();
}

/**************************************************************************/

void Pupil::FullScreen(Fl_Window* fsw)
{
  if(bFullScreen) {
    if(fsw == mFullScreenWin)
       fsw->fullscreen_off(mFSx, mFSy, mFSw, mFSh);
    mFullScreenWin = 0;
  } else {
    mFSx = fsw->x(); mFSy = fsw->y(); mFSw = fsw->w(); mFSh = fsw->h();
    fsw->fullscreen();
    mFullScreenWin = fsw;
  }
  bFullScreen = !bFullScreen;
}

/**************************************************************************/
// Render
/**************************************************************************/

void Pupil::Render(bool rnr_self, bool rnr_overlay)
{
  // Calls rnr driver to perform actual rendering.
  // Used by draw() and Pick().

  static const Exc_t _eh("Pupil::Render ");

  mDriver->SetWidthHeight(w(), h());
  mDriver->SetZFov(mInfo->GetZFov());
  mDriver->SetNearClip(mInfo->GetNearClip());
  mDriver->SetFarClip (mInfo->GetFarClip());
  mDriver->SetMaxDepth(mInfo->GetMaxRnrDepth());

  PupilInfo_GL_Rnr* pupil_rnr = dynamic_cast<PupilInfo_GL_Rnr*>(mDriver->GetRnr(fImg));
  pupil_rnr->InitRendering(mDriver);
  mDriver->BeginRender();
  if (rnr_self)
  {
    try
    {
      mDriver->Render(pupil_rnr);
    }
    catch(Exc_t exc)
    {
      cout << _eh << "scene exception: '" << exc << "'.\n";
    }
  }
  if (bShowOverlay && rnr_overlay && mOverlayImg)
  {
    try
    {
      // !!!! Manually call pupil_rnr->Pre/PostDraw().
      // Somewhat hacky ... but can happen that handle is called
      // before the first draw is done and then font is undefined.
      // Should do this in GLRnrDriver::BeginRender()?
      pupil_rnr->PreDraw(mDriver);
      mDriver->Render(mDriver->GetRnr(*mOverlayImg));
      pupil_rnr->PostDraw(mDriver);
    }
    catch (Exc_t exc)
    {
      cout << _eh << "overlay exception: '" << exc << "'.\n";
    }
  }
  mDriver->EndRender();

  if (mDriver->SizePM() > 0)
  {
    printf("%sposition stack not empty (%d).\n", _eh.Data(), mDriver->SizePM());
    mDriver->ClearPM();
  }

  GLenum gl_err = glGetError();
  if (gl_err)
  {
    printf("%sGL error: %s.\n", _eh.Data(), gluErrorString(gl_err));
  }

}

/**************************************************************************/
// Picking
/**************************************************************************/

void Pupil::cam_towards_cb(Fl_Widget* w, Pupil::pick_menu_data* ud)
{
  ud->pupil->TurnCamTowards(ud->img->fLens, ud->pupil->default_distance());
}

void Pupil::cam_at_cb(Fl_Widget* w, Pupil::pick_menu_data* ud)
{
  ud->pupil->TurnCamTowards(ud->img->fLens, 0);
}

void Pupil::copy_to_clipboard_cb(Fl_Widget* w, Pupil::pick_menu_data* ud)
{
  ud->pupil->GetShell()->X_SetSource(ud->img);
  const char* idstr = GForm("%u", ud->img->fLens->GetSaturnID());
  Fl::copy(idstr, strlen(idstr), 0);
}

void Pupil::deliver_event_cb(Fl_Widget* w, Pupil::pick_menu_data* ud)
{
  printf("deliver event activated ... size=%zu\n", ud->name_stack.size());
  ud->name_stack_iterator->fRnr->HandlePick(ud->pupil->mDriver,
					    ud->name_stack,
					    ud->name_stack_iterator);
}

void Pupil::fill_pick_menu(A_Rnr::lNSE_t& ns, A_Rnr::lNSE_i nsi,
			   OS::ZGlassImg* img, Fl_Menu_Button& menu,
			   FTW_Shell::mir_call_data_list& mcdl,
			   const TString& prefix)
{
  mcdl.push_back(new pick_menu_data(this, ns, nsi, img));

  menu.add(GForm("%sJump Towards", prefix.Data()),
	   0, (Fl_Callback*)cam_towards_cb, mcdl.back(), 0);
  menu.add(GForm("%sJump At", prefix.Data()),
	   0, (Fl_Callback*)cam_at_cb, mcdl.back(), 0);
  menu.add(GForm("%sDeliver Event", prefix.Data()),
	   0, (Fl_Callback*)deliver_event_cb, mcdl.back(), FL_MENU_DIVIDER);
}


Int_t Pupil::Pick(int xpick, int ypick, bool rnr_self, bool rnr_overlay)
{
  make_current();

  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPickMatrix((GLdouble) xpick, (GLdouble)(vp[3] - ypick),
		mInfo->GetPickR(), mInfo->GetPickR(), vp);
  SetProjection();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetCameraView();

  if (mPickBuff == 0) mPickBuff = new UInt_t[mPickBuffSize];
  glSelectBuffer((GLsizei)mPickBuffSize, (GLuint*)mPickBuff);
  glRenderMode(GL_SELECT);
  mDriver->BeginPick();
  Render(rnr_self, rnr_overlay);
  mDriver->EndPick();
  GLint n = glRenderMode(GL_RENDER);

  if (n < 0)
    printf("Pupil::Pick overflow of selection buffer, %d entries returned.\n", (int)n);

  return n;
}

Int_t Pupil::PickTopNameStack(A_Rnr::lNSE_t& result, float& min_z, float& max_z,
			      int  xpick,    int  ypick,
			      bool rnr_self, bool rnr_overlay)
{
  Int_t n = Pick(xpick, ypick, rnr_self, rnr_overlay);

  if (n > 0)
  {
    min_z = 1e10;
    UInt_t *min_p = 0;
    UInt_t *x     = mPickBuff;

    for (int i = 0; i < n; ++i)
    {
      float zmin = (float) *(x+1)/0x7fffffff;
      if (zmin < min_z)
      {
	min_z = zmin;
	max_z = (float) *(x+2)/0x7fffffff;
	min_p = x;
      }
      x += 3 + *x;
    }
    if (min_p == 0) return 0;

    x = min_p;
    int m = *x; x += 3;
    for (int i = 0; i < m; ++i)
    {
      result.push_front(mDriver->NameStack(x[i]));
    }
  }
  return n;
}

Int_t Pupil::PickLenses(list<pick_lens_data>& result,
			bool sort_z,   bool fill_stack,
			int  xpick,    int  ypick,
			bool rnr_self, bool rnr_overlay)
{
  // Fill the 'result' list with data obtained from GL.
  // See definition of the struct pick_lens_data in the header file.
  // The user-data information passed to the GL-render driver is lost.

  static const Exc_t _eh("Pupil::PickLenses ");

  Int_t n = Pick(xpick, ypick, rnr_self, rnr_overlay);

  if (n > 0)
  {
    UInt_t* x = mPickBuff;
    for (int i = 0; i < n; ++i)
    {
      UInt_t m = *x; x++;

      if (m == 0) {
	// This is ok ... name-stack ops can be switched off.
	x += 2 + m;
	continue;
      }
      if (x - mPickBuff + 2 + m > mPickBuffSize) {
	cout << _eh << "overflow of selection buffer, should not happen.\n";
	x += 2 + m;
	continue;
      }

      float zmin = (float) *x/0x7fffffff;
      x++;
      // float zmax = (float) *x/0x7fffffff;
      x++;

      UInt_t id = x[m-1];

      OS::ZGlassImg *root_img = mDriver->NameStack(id).fRnr->fImg;
      ZGlass        *lens     = root_img->fLens;

      pick_lens_data pld(root_img, zmin, lens->GetName());;

      if (fill_stack)
      {
	pld.name_stack.push_back(mDriver->NameStack(id));
	for (int j = m-2; j >= 0; --j) {
	  UInt_t p_id = x[j];
	  pld.name_stack.push_back(mDriver->NameStack(p_id));
	}
      }

      if (sort_z) {
	list<pick_lens_data>::iterator ins_pos = result.begin();
	while (ins_pos != result.end() && ins_pos->z < pld.z) {
	  ++ins_pos;
	}
	result.insert(ins_pos, pld);
      } else {
	result.push_back(pld);
      }

      x += m;
    }
  }

  return n;
}

OS::ZGlassImg* Pupil::PickTopLens(int xpick, int ypick,
				  bool rnr_self, bool rnr_overlay)
{
  // Return image of the closest lens that matches selection.

  list<pick_lens_data> gdl;
  Int_t n = PickLenses(gdl, true, false, xpick, ypick, rnr_self, rnr_overlay);
  return (n > 0) ? gdl.front().img : 0;
}

/**************************************************************************/

void Pupil::PickMenu(int xpick, int ypick, bool rnr_self, bool rnr_overlay)
{
  // Perform picking via PickLenses() and construct context menu for
  // intersected lenses.

  list<pick_lens_data> gdl;
  Int_t n = PickLenses(gdl, true, true, xpick, ypick, rnr_self, rnr_overlay);

  if (n > 0)
  {
    Fl_Menu_Button menu(Fl::event_x(), Fl::event_y(), 0, 0, 0);
    menu.textsize(mShell->cell_fontsize());
    FTW_Shell::mir_call_data_list mcdl;

    int loc = 1;
    for (list<pick_lens_data>::iterator gdi = gdl.begin();
	 gdi != gdl.end(); ++gdi)
    {
      if (mInfo->GetPickDisp() != 0)
      {
	Float_t near  = mInfo->GetNearClip();
	Float_t far   = mInfo->GetFarClip();
	Float_t zdist = near*far/(far - gdi->z/2*(far - near));
	if (mInfo->GetPickDisp() == 1)
	  gdi->name = GForm("%2d. (%6.3f)  %s/",  loc, zdist, gdi->name.Data());
	else
	  gdi->name = GForm("%2d. (%6.3f%%)  %s/", loc, 100*(zdist/far), gdi->name.Data());
      }
      else
      {
	gdi->name = GForm("%2d. %s/", loc, gdi->name.Data());
      }

      fill_pick_menu(gdi->name_stack, gdi->name_stack.begin(), gdi->img,
		     menu, mcdl, gdi->name);
      mShell->FillLensMenu(gdi->img, menu, mcdl, gdi->name);
      mShell->FillShellVarsMenu(gdi->img, menu, mcdl, gdi->name);

      // iterate through the list of parents
      menu.add(GForm("%sParents", gdi->name.Data()), 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER);
      A_Rnr::lNSE_i pe = gdi->name_stack.begin();
      ++pe; // skip top entry
      while (pe != gdi->name_stack.end())
      {
	OS::ZGlassImg *img = pe->fRnr->fImg;
	TString entry(GForm("%sParents/%s/", gdi->name.Data(), img->fLens->GetName()));
	fill_pick_menu(gdi->name_stack, pe, img, menu, mcdl, entry);
	mShell->FillLensMenu(img, menu, mcdl, entry);
	mShell->FillShellVarsMenu(img, menu, mcdl, entry);
	++pe;
      }

      ++loc;
      if (loc > mInfo->GetPickMaxN()) break;
    }

    menu.popup();
  }
}

/**************************************************************************/
// fltk stuff
/**************************************************************************/

void Pupil::label_window(const char* l)
{
  if(l == 0) l = GForm("pupil: %s '%s'", mInfo->GetName(), mInfo->GetTitle());
  FTW_SubShell::label_window(l);
}

/**************************************************************************/
// fltk draw
/**************************************************************************/

void Pupil::draw()
{
  static const Exc_t _eh("Pupil::draw ");

  if(glew_init_needed) {
#ifdef __APPLE__
    // We use GLEW from ROOT which would also try to init GLX -- avoid that.
    // GLenum status = glewContextInit();
    // Not anymore ... but ROOT should stop using GLX ...
    GLenum status = glewInit();
#else
    GLenum status = glewInit();
#endif
    if (status != GLEW_OK)
      cout << "glewInit - GLEW initalization failed.\n";
    else
      cout << "glewInit - GLEW initalization successful.\n";
    glew_init_needed = false;
  }

  // if (!valid()) {}

  if (mCreationThread == 0)
    mCreationThread = GThread::Self();

  if (bDumpImage)
  {
    try {
      if (mFBO == 0)
      {
	mFBO = new FBO();
      }
      if (mFBO->fW != w() || mFBO->fH != h())
        mFBO->init(w(), h());
    }
    catch(const char* exc) {
      printf("%sFBO init failed: %s.\n", _eh.Data(), exc);
    }

    if (mImgNTiles > 1)
    {
      for (Int_t xi=0; xi<mImgNTiles; ++xi)
      {
	for (Int_t yi=0; yi<mImgNTiles; ++yi)
	{
          mFBO->bind();
	  rnr_standard(false, mImgNTiles, xi, yi);
	  TString fname(GForm("%s-%d-%d.tga", mImageName.Data(), yi, xi));
	  dump_image(fname);
          mFBO->unbind();
	}
      }
      printf("Merge with: montage -tile %dx%d -geometry %dx%d %s-* %s.png\n",
	     mImgNTiles, mImgNTiles, w(), h(), mImageName.Data(), mImageName.Data());
    }
    else
    {
      mFBO->bind();
      rnr_standard(mInfo->bRnrFakeOverlayInCapture);
      dump_image(mImageName + ".tga");
      mFBO->unbind();

      if (bCopyToScreen)
      {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {
          // rnr_default_init();

          glDisable(GL_LIGHTING);
          glDisable(GL_BLEND);
          glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
          glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
          mFBO->bind_texture();
          glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
          glBegin(GL_QUADS);
          glTexCoord2f(0,0);  glVertex3f(-1, -1, -1);
          glTexCoord2f(1,0);  glVertex3f( 1, -1, -1);
          glTexCoord2f(1,1);  glVertex3f( 1,  1, -1);
          glTexCoord2f(0,1);  glVertex3f(-1,  1, -1);
          glEnd();
          mFBO->unbind_texture();
          glMatrixMode(GL_PROJECTION); glPopMatrix();
          glMatrixMode(GL_MODELVIEW);  glPopMatrix();
        }

        bCopyToScreen = false;
      }
    }

    bDumpImage = false;
  }
  else
  {
    if (bStereo)
    {
      rnr_stereo(true);
    }
    else
    {
      rnr_standard(true);
    }
  }

  if (bSignalDumpFinish)
  {
    mInfo->ReceiveDumpFinishedSignal();
    bSignalDumpFinish = false;
  }

  check_driver_redraw();
}

/**************************************************************************/

void Pupil::rnr_standard(bool rnr_overlay_p, Int_t n_tiles, Int_t x_i, Int_t y_i)
{
  static const Exc_t _eh("Pupil::rnr_standard ");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w(), h());
  SetProjection(n_tiles, x_i, y_i);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  try
  {
    SetAbsRelCamera();
    SetCameraView();
  }
  catch(Exc_t& exc) {
    cerr << _eh << "exception during camera setup: " << exc << endl;
  }

  rnr_default_init();

  GTime start_time(GTime::I_Now);
  Render();
  GTime stop_time(GTime::I_Now);

  if (rnr_overlay_p)
  {
    GTime rnr_time = stop_time - start_time;
    rnr_fake_overlay(rnr_time);
  }
}

void Pupil::rnr_stereo(bool rnr_overlay_p)
{
  static const Exc_t _eh("Pupil::rnr_stereo ");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w(), h());
  SetProjection();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  try
  {
    SetAbsRelCamera();
    SetCameraView();
  }
  catch(Exc_t& exc) {
    cerr << _eh << "exception during camera setup: " << exc << endl;
  }

  Double_t zero_p_dist   = mFNear + mInfo->GetStereoZeroParallax() * (mFFar - mFNear);
  Double_t x_len_zero_p  = (mFRgt - mFLft) * zero_p_dist / mFNear;
  Double_t stereo_offset = 0.035 * x_len_zero_p * mInfo->GetStereoEyeOffsetFac();
  Double_t frustum_asym  = stereo_offset * mFNear / zero_p_dist * mInfo->GetStereoFrustumAsymFac();

  GTime start_time(GTime::I_Now);

  const Double_t *left_vec = mCamAbsTrans.ArrY();

  // --- Left ---

  glDrawBuffer(GL_BACK_LEFT);

  glPushMatrix();
  glTranslated(stereo_offset*left_vec[0], stereo_offset*left_vec[1], stereo_offset*left_vec[2]);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixd(mProjBase.Array());
  glFrustum(mFLft + frustum_asym, mFRgt + frustum_asym,
	    mFBot, mFTop, mFNear, mFFar);
  glMatrixMode(GL_MODELVIEW);

  rnr_default_init();
  Render();

  glPopMatrix();

  // --- Right ---

  glDrawBuffer(GL_BACK_RIGHT);

  glPushMatrix();
  glTranslated(-stereo_offset*left_vec[0], -stereo_offset*left_vec[1], -stereo_offset*left_vec[2]);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixd(mProjBase.Array());
  glFrustum(mFLft - frustum_asym, mFRgt - frustum_asym,
	    mFBot, mFTop, mFNear, mFFar);
  glMatrixMode(GL_MODELVIEW);

  rnr_default_init();
  Render();

  glPopMatrix();

  // --- End stuff

  GTime stop_time(GTime::I_Now);

  if (rnr_overlay_p)
  {
    GTime rnr_time = stop_time - start_time;

    glDrawBuffer(GL_BACK_LEFT);
    rnr_fake_overlay(rnr_time);

    glDrawBuffer(GL_BACK_RIGHT);
    rnr_fake_overlay(rnr_time);
  }
}

void Pupil::rnr_default_init()
{
  static const GLfloat mat_diffuse[]   = { 1.0, 1.0, 1.0, 1.0 };
  static const GLfloat mat_specular[]  = { 0.2, 0.2, 0.2, 1.0 };
  static const GLfloat mat_shininess[] = { 60.0 };

  { // clear
    const ZColor& c = mInfo->RefClearColor();
    glClearColor(c.r(), c.g(), c.b(), c.a());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Should move this shit somewhere ... and hope other shit cleans up ... naaah
  // i think i would better leave it here a bit longer
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  // This is overriden with first light ... just in case there are no lights in the scene
  GLfloat ambient[]  = { 0.1, 0.1,  0.1, 1.0 };
  GLfloat position[] = { 0.0, 0.0, 10.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glEnable(GL_LIGHT0);

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, mInfo->GetLiMo2Side());
  glPolygonMode(GL_FRONT, (GLenum)mInfo->GetFrontMode());
  glPolygonMode(GL_BACK,  (GLenum)mInfo->GetBackMode());

  glPointSize(1);
  glLineWidth(1);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  if(mInfo->GetBlend()) {
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND);
  }
  glDisable(GL_FOG);
}

void Pupil::rnr_fake_overlay(GTime& rnr_time)
{
  // Overlay simulation ... couldn't get draw_overlay working
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  float aspect = (float)w()/h();
  glScalef(1, aspect, 1);

  GLfloat ch_size = mInfo->GetCHSize();
  if(ch_size > 0) {
    glLineWidth(1);
    glColor3f(1,0,0);
    glBegin(GL_LINES); {
      glVertex2f(ch_size, 0);	 glVertex2f(ch_size/3,0);
      glVertex2f(-ch_size/3, 0); glVertex2f(-ch_size,0);
      glVertex2f(0, ch_size);	 glVertex2f(0,ch_size/3);
      glVertex2f(0, -ch_size/3); glVertex2f(0,-ch_size);
    } glEnd();
  }

  GLfloat mp_size = mInfo->GetMPSize();
  if(bMPIn && mp_size > 0) {
    glColor3f(0, 0.5, 0.8);
    glPushMatrix();
    glTranslatef(2.0*mMPX/w() - 1, (1 - 2.0*mMPY/h())/aspect, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0);
    glVertex2f(0.7*mp_size ,-mp_size);
    glVertex2f(mp_size, -mp_size);
    glVertex2f(mp_size, -0.7*mp_size);
    glEnd();
    glPopMatrix();
  }

  if(mInfo->GetShowRPS() == true) {
    const TString text(GForm("%.1frps", TMath::Min(1/rnr_time.ToDouble(), 999.9)));
    ZColor col = ZColor(1,1,1) - mInfo->RefClearColor();
    GLTextNS::RnrTextAt(mDriver, text, 2, 0, 1e-3, &col, 0);
  }
  if(mInfo->GetShowView() == true) {
    const char* base = "<null>";
    if(mInfo->GetCameraBase()) base = mInfo->GetCameraBase()->GetName();
    const char *mode, *zstr; float zval;
    if(mInfo->GetProjMode() == PupilInfo::P_Perspective) {
      mode = "persp"; zstr = "zfov"; zval = mInfo->GetZFov();
    } else {
      mode = "orto"; zstr = "zsize"; zval = mInfo->GetZSize();
    }
    ZTrans& z( mCamAbsTrans );
    int omw = int(mInfo->GetMoveOM()) - 2;
    omw = (omw < 0) ? -omw : 0;

    const TString text1
      (GForm("mode='%s' %s=%.2f clip=(%.3f,%.3f)",
	     mode, zstr, zval,
	     mInfo->GetNearClip(), mInfo->GetFarClip()));
    const TString text2
      (GForm("base='%1$s' pos=(%2$.*5$f,%3$.*5$f,%4$.*5$f)",
	     base, z(1,4), z(2,4), z(3,4), omw));

    ZColor col = ZColor(1,1,1) - mInfo->RefClearColor();
    GLTextNS::RnrTextAt(mDriver, text1, -2, 0, 1e-3, &col, 0);
    GLTextNS::RnrTextAt(mDriver, text2, -2, 1, 1e-3, &col, 0);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

/**************************************************************************/
// fltk handle
/**************************************************************************/

void Pupil::setup_rnr_event(int ev, A_Rnr::Fl_Event& e)
{
  e.fEvent   = ev;

  e.fState   = Fl::event_state();
  e.fKey     = Fl::event_key();
  e.fButton  = Fl::event_button();
  e.fButtons = Fl::event_buttons();
  e.fClicks  = Fl::event_clicks();
  e.fIsClick = Fl::event_is_click();
  e.fX       = Fl::event_x();
  e.fY       = Fl::event_y();
  e.fDX      = Fl::event_dx();
  e.fDY      = Fl::event_dy();
  e.fText    = TString(Fl::event_text(), Fl::event_length());

  e.fIsKey   = (ev == FL_KEYDOWN || ev == FL_KEYUP);

  e.fIsMouse = (ev == FL_ENTER || ev == FL_MOVE || ev == FL_LEAVE ||
		ev == FL_PUSH  || ev == FL_DRAG || ev == FL_RELEASE ||
                ev == FL_MOUSEWHEEL);

  e.fIsOverlay  = false;
  e.fCurrentNSE = e.fNameStack.end();
  e.fZMin = e.fZMax = 0;

  // Fltk sometimes still sends keyup events from auto-repeat --
  // transform them to keydown. Apparently this only happens when
  // running with multiple threads as I could not reproduce this
  // behaviour in a minimal fltk program.
  //
  // This requires round trip to X server, so the thing should be fixed
  // in fltk (or wherever).
  if (ev == FL_KEYUP && Fl::get_key(e.fKey))
    e.fEvent = FL_KEYDOWN;
}

int Pupil::overlay_pick(A_Rnr::Fl_Event& e)
{
  Int_t n = PickTopNameStack(e.fNameStack, e.fZMin, e.fZMax, e.fX, e.fY, false, true);
  if (n > 0)
  {
    e.fCurrentNSE    = e.fNameStack.begin();
    e.fNameStackSize = n;
  }
  else
  {
    e.fCurrentNSE    = e.fNameStack.end();
    e.fNameStackSize = 0;
  }
  return n;
}

int Pupil::overlay_pick_and_deliver(A_Rnr::Fl_Event& e)
{
  Int_t n = overlay_pick(e);
  if (n > 0)
  {
    while (e.fCurrentNSE != e.fNameStack.end())
    {
      if (e.fCurrentNSE->fRnr->Handle(mDriver, e))
	return 1;
      ++e.fCurrentNSE;
    }
  }
  return 0;
}

int Pupil::handle_overlay(A_Rnr::Fl_Event& e)
{
  int ev = e.fEvent;

  if (ev == FL_ENTER)
  {
    if (overlay_pick_and_deliver(e))
    {
      mDriver->SetBelowMouse(e.fCurrentNSE->fRnr);
    }
    // handle() will always return 1 on FL_ENTER to keep getting move events.
    // We return 0 so that event-handler can get it, too.
    return 0;
  }
  else if (ev == FL_LEAVE)
  {
    if (mDriver->GetBelowMouse())
    {
      mDriver->GetBelowMouse()->Handle(mDriver, e);
      mDriver->SetBelowMouse(0);
    }
    // handle() will always return 1 on FL_LEAVE.
    // We return 0 so that event-handler can get it, too.
    return 0;
  }
  else if (ev == FL_MOVE)
  {
    A_Rnr* below_mouse = mDriver->GetBelowMouse();
    Int_t n = PickTopNameStack(e.fNameStack, e.fZMin, e.fZMax, e.fX, e.fY, false, true);
    if (n > 0)
    {
      // Simulate ENTER / LEAVE events.
      e.fEvent = FL_ENTER;
      e.fCurrentNSE = e.fNameStack.begin();
      while (e.fCurrentNSE != e.fNameStack.end() &&
	     e.fCurrentNSE->fRnr != below_mouse)
      {
	if (e.fCurrentNSE->fRnr->Handle(mDriver, e))
	{
	  if (below_mouse)
	  {
	    e.fEvent = FL_LEAVE;
	    below_mouse->Handle(mDriver, e);
	  }
	  mDriver->SetBelowMouse(e.fCurrentNSE->fRnr);
	  return 1;
	}
	++e.fCurrentNSE;
      }
      // Deliver MOVE
      e.fEvent = FL_MOVE;
      if (below_mouse)
      {
	return below_mouse->Handle(mDriver, e);
      }
    }
    else
    {
      if (below_mouse)
      {
	e.fEvent = FL_LEAVE;
	below_mouse->Handle(mDriver, e);
	mDriver->SetBelowMouse(0);
	return 1;
      }
    }
    return 0;
  }
  else if (ev == FL_PUSH || ev == FL_RELEASE || ev == FL_DRAG)
  {
    A_Rnr* pushed = mDriver->GetPushed();
    if (pushed)
    {
      overlay_pick(e);
      pushed->Handle(mDriver, e);
      if (ev == FL_RELEASE) // as in fltk; would prefer "&& e.fButtons == 0"
      {
	mDriver->SetPushed(0);
      }
      return 1;
    }
    else
    {
      if (ev == FL_PUSH)
      {
	if (overlay_pick_and_deliver(e))
	{
	  mDriver->SetPushed(e.fCurrentNSE->fRnr);
	  return 1;
	}
      }
    }
  }
  else if (ev == FL_MOUSEWHEEL)
  {
    if (mDriver->GetBelowMouse())
      return mDriver->GetBelowMouse()->Handle(mDriver, e);
  }
  else if (ev == FL_KEYDOWN || ev == FL_KEYUP)
  {
    if (mDriver->GetBelowMouse())
      return mDriver->GetBelowMouse()->Handle(mDriver, e);
  }

  return 0;
}

int Pupil::handle(int ev)
{
  static const Exc_t _eh("Pupil::handle ");

  int x = Fl::event_x(); int y = Fl::event_y();
  // printf("PupilEvent %d (%d, %d)\n", ev, x, y);

  if (ev == FL_ENTER)
  {
    // Button events also cause enter/leave due to pointer grab (at least on X)
#if !defined(__APPLE__) and !defined(WIN32)
    if (fl_xevent->xcrossing.mode != 0)
      return 1;
#endif
    mMPX = x; mMPY = y;
    bMPIn = true;
  }
  if (ev == FL_LEAVE)
  {
    // Button events also cause enter/leave due to pointer grab (at least on X)
#if !defined(__APPLE__) and !defined(WIN32)
    if (fl_xevent->xcrossing.mode != 0)
      return 1;
#endif
    bMPIn = false;
  }
  if (ev == FL_MOVE && mInfo->mMPSize > 0)
  {
    mMPX = x; mMPY = y;
    redraw();
  }

  // Check for Ctrl-` -- toggle external event handler.
  if (ev == FL_KEYBOARD && Fl::event_key() == '`' && Fl::event_state(FL_CTRL) &&
     mInfo->bAllowHandlerSwitchInPupil)
  {
    bUseEventHandler = !bUseEventHandler;
    return 1;
  }

  if (ev == FL_HIDE || ev == FL_SHOW)
  {
    return Fl_Gl_Window::handle(ev);
  }

  // Maybe should check for something else?
  if (!valid())
  {
    return 1;
  }

  // It can happen that we get called from another thread as ~Fl_Widget()
  // calls fl_throw_focus() which in turn calls handle() on current widget.
  // This seems utterly wrong.
  if (GThread::Self() != mCreationThread)
  {
    // printf("%scalled from *another* thread:\n", _eh.Data());
    // printf("  creation='%s' calling='%s'.\n", mCreationThread->GetName(), GThread::Self()->GetName());
    return 0;
  }

  make_current();

  A_Rnr::Fl_Event e;
  setup_rnr_event(ev, e);
  mDriver->PreEventHandling(e);

  // Call handle in PupilInfo_GL_Rnr ... this should mostly return 0
  // but can process some keyboard shortcuts.
  // It also copies some variables to the other side.
  {
    if (mDriver->GetRnr(fImg)->Handle(mDriver, e))
      return 1;
  }

  if (mOverlayImg && bShowOverlay)
  {
    try
    {
      e.fIsOverlay = true;
      if (handle_overlay(e))
      {
	check_driver_redraw();
	return 1;
      }
      // Restore event-type - handle_overlay() can change MOVE to ENTER/LEAVE.
      e.fEvent     = ev;
      e.fIsOverlay = false;
    }
    catch (Exc_t exc)
    {
      printf("%sexception in handle_overlay: '%s'.\n", _eh.Data(), exc.Data());
      return 1;
    }
  }

  if (mEventHandlerImg && bUseEventHandler)
  {
    if (mDriver->GetRnr(*mEventHandlerImg)->Handle(mDriver, e))
      return 1;
  }

  if (ev == FL_ENTER || ev == FL_LEAVE)
  {
    return 1;
  }

  if (ev == FL_SHORTCUT && Fl::event_key() == FL_Escape && parent() == 0)
  {
    iconize();
    return 1;
  }

  switch(ev)
  {
    case FL_PUSH:
    {
      mMouseX = x; mMouseY = y; // reset the drag location

      // Raise on click.
      if(Fl::event_button() == 1) {
	show();
      }

      if(Fl::event_button() == 1 && Fl::event_clicks() == 1) {
	Fl::event_clicks(0);
	OS::ZGlassImg* img = PickTopLens(mMouseX, mMouseY);
	if(img) {
	  int x = Fl::event_x_root() + mInfo->GetPopupDx();
	  int y = Fl::event_y_root() + mInfo->GetPopupDy();
	  mShell->SpawnMTW_View(img, true, true,
				x, y, mInfo->GetPopupFx(), mInfo->GetPopupFy());
	}
      }

      if(Fl::event_button() == 3) {
	PickMenu(mMouseX, mMouseY);
      }

      return 1;
    }

    case FL_DRAG:
    {
      bool chg = 0;
      int dx = x - mMouseX, dy = y - mMouseY;
      mMPX += dx; mMPY += dy;
      // Invert dx/dy from "screen" to "camera" coordinates.
      dx = -dx; dy = -dy;
      mMouseX = x; mMouseY = y;
      Float_t move_fac = mInfo->GetMSMoveFac() *
	TMath::Power(10, mInfo->GetMoveOM());
      Float_t rot_fac  = mInfo->GetMSRotFac() * TMath::TwoPi() / 1000;
      if(Fl::event_state(FL_BUTTON1)) {
	double Dy = dy*move_fac*TMath::Power(abs(dy), mInfo->GetAccelExp());
	if(Dy != 0) {
	  chg = 1;
	  if(!Fl::event_state(FL_CTRL)) {
	    mCamera->MoveLF(1, Dy);
	  } else {
	    mCamera->MoveLF(3, Dy);
	  }
	}
	double Dx = dx*move_fac*TMath::Power(abs(dx), mInfo->GetAccelExp());
	if(Dx != 0) {
	  chg = 1;
	  mCamera->MoveLF(2, Dx);
	}
      }

      if(Fl::event_state(FL_BUTTON2) && dy!=0) {
	if(!Fl::event_state(FL_CTRL)) {
	  mCamera->RotateLF(1,3,dy*rot_fac);
	} else {
	  mCamera->RotateLF(1,3,-dy*rot_fac);
	}
	chg=1;
      }
      if(Fl::event_state(FL_BUTTON2) && dx!=0) {
	if(!Fl::event_state(FL_CTRL)) {
	  mCamera->RotateLF(1, 2, dx*rot_fac);
	} else {
	  mCamera->RotateLF(2, 3, -dx*rot_fac);
	}
	chg=1;
      }
      if(chg) redraw();
      return 1;
    }

    case FL_MOUSEWHEEL:
    {
      if(Fl::event_dy() != 0) {
	Float_t delta = -Fl::event_dy();
	Float_t fac = mInfo->GetZoomFac();
	if(fac != 1) {
	  if(Fl::event_state(FL_CTRL))       fac = TMath::Power(fac, 0.25);
	  else if(Fl::event_state(FL_SHIFT)) fac = TMath::Power(fac, 4);
	  auto_ptr<ZMIR> mir( mInfo->S_ZoomFac(TMath::Power(fac, delta)) );
	  mShell->Send(*mir);
	} else {
	  if(Fl::event_state(FL_CTRL))       delta *= 0.2;
	  else if(Fl::event_state(FL_SHIFT)) delta *= 5;
	  auto_ptr<ZMIR> mir( mInfo->S_Zoom(delta) );
	  mShell->Send(*mir);
	}
      }
      return 1;
    }

    case FL_KEYBOARD:
    {
      switch(Fl::event_key())
      {
	case FL_Home:
	{
	  bool smooth = Fl::event_state(FL_SHIFT);
	  auto_ptr<ZMIR> mir( mInfo->S_Home(smooth) );
	  mShell->Send(*mir);
	  return 1;
	}
	case FL_Tab:
	{
	  bShowOverlay = !bShowOverlay; redraw();
	  return 1;
	}
	case 'f':
	case FL_F+12:
	{
	  Fl_Group* fsg = this;
	  while(fsg->parent()) fsg = fsg->parent();
	  if(fsg->type() >= FL_WINDOW) {
	    Fl_Window* fsw = (Fl_Window*)fsg;
	    FullScreen(fsw);
	    fsw->redraw();
	  }
	  return 1;
	}
	case FL_F+1:
	{
	  mShell->SpawnMTW_View(fImg, true, true);
	  return 1;
	}
	case FL_F+2:
	{
	  if(mCameraView == 0) {
	    FTW_Window* w = new FTW_Window(0,0);
	    MTW_ClassView* cv = new MTW_ClassView(mCamera, mShell);
	    mCameraView = cv;
	    w->end();
	    cv->BuildVerticalView();
	    mShell->adopt_window(w);
	    mCamera->register_ray_absorber(&mCameraCB);
	  }
	  mCameraView->GetWindow()->show();
	  return 1;
	}
      } // end switch(Fl::event_key())

      return 1;
    } // case FL_KEYBOARD

  } // switch(ev)

  return 0;
}

/**************************************************************************/
// Protected methods.
/**************************************************************************/

void Pupil::dump_image(const TString& fname)
{
  static const Exc_t _eh("Pupil::dump_image ");

  printf("%sdumping '%s'.\n", _eh.Data(), fname.Data());

  FILE* img = fopen(fname.Data(), "w");
  if(img == 0) {
    printf("%scan't open screenshot file '%s'.\n", _eh.Data(), fname.Data());
    return;
  }

  tga_header header(w(), h());
  header.dump(img);

  unsigned char* xx = new unsigned char[w()*h()*3];
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, w(), h(), GL_BGR, GL_UNSIGNED_BYTE, xx);
  fwrite(xx, 3, w()*h(), img);
  delete [] xx;

  fclose(img);
}

/**************************************************************************/
// Driver redraw
/**************************************************************************/

void Pupil::check_driver_redraw()
{
  if(bAutoRedraw && mDriver->GetRedraw()) {
    Fl::add_timeout(0, (Fl_Timeout_Handler)redraw_timeout, this);
  }
}

void Pupil::redraw_timeout(Pupil* pup)
{
  pup->redraw();
}

/**************************************************************************/
// Hacks.
/**************************************************************************/

float Pupil::default_distance()
{
  return mInfo->GetMSMoveFac() * TMath::Power(10, mInfo->GetMoveOM() + 2);
}

void Pupil::camera_stamp_cb::AbsorbRay(Ray& ray)
{
  pupil->mCameraView->UpdateDataWeeds(FID_t(0,0));
  pupil->redraw();
}

/**************************************************************************/

void Pupil::initiate_smooth_camera_home()
{
  GLensReadHolder _rdlck(mInfo);

  smooth_camera_home_data* data = new smooth_camera_home_data;
  data->pupil    = this;
  data->distance = mCamera->ref_trans().GetPos().Mag();
  data->time     = -0.999;
  data->delta_t  = 2.0/(25*mInfo->GetHomeAnimTime());
  Fl::add_timeout(0, (Fl_Timeout_Handler)smooth_camera_home_cb, data);
}

void Pupil::smooth_camera_home_cb(smooth_camera_home_data* data)
{
  static const float fps_25_dt = 0.04;
  Pupil* P = data->pupil;

  GLensReadHolder _rdlck(P->mInfo);

  if(data->time >= 1) {
    P->mCamera->Home(); delete data;
  } else {
    float    t = data->time;
    float  pos = data->distance*(1 - 0.75*((t - t*t*t/3) + 0.666666667));
    ZTrans&  T = P->mCamera->ref_trans();
    ZPoint   v = T.GetPos();
    Double_t m2 = v.Mag2();
    // printf("t=%f pos=%f dist=%f m2=%lf\n", t, pos, data->distance, m2);
    if(m2 < 1e-5) {
      delete data; P->mCamera->Home();
    } else {
      v *= pos/TMath::Sqrt(m2);
      T.SetPos(v);
      data->time += data->delta_t;
      Fl::repeat_timeout(fps_25_dt, (Fl_Timeout_Handler)smooth_camera_home_cb, data);
    }
  }
  P->redraw();
}

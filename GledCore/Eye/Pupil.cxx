// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Pupil.h"
#include "Eye.h"
#include "FTW_Shell.h"

#include <Glasses/Camera.h>
#include <Glasses/PupilInfo.h>
#include <Gled/GledNS.h>
#include <Ephra/Saturn.h>

#include <TVirtualX.h>
#include <TGX11.h>
#include <TSystem.h>
#include <TVector3.h>

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/x.H>
#include <X11/Xlib.h>

#include <FL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <math.h>

namespace OS = OptoStructs;

/**************************************************************************/

namespace {

  short ss(short s) {
    short r; char* c = (char*)&s, *b = (char*)&r;;
    b[0] = c[1]; b[1] = c[0];
    return r;
  }

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
      TBuffer b(TBuffer::kWrite);
      b << identsize << colourmaptype << imagetype;
      b << ss(colourmapstart) << ss(colourmaplength) << colourmapbits;
      b << ss(xstart) << ss(ystart) << ss(width) << ss(height);
      b << bits << descriptor;
      fwrite(b.Buffer(), 1, b.Length(), fp);
    }
  }; 
}

/**************************************************************************/

void Pupil::label_window()
{
  mLabel = GForm("pupil: %s; %s", mInfo->GetName(), mInfo->GetTitle());
  label(mLabel.c_str());
  redraw();
}

/**************************************************************************/

void Pupil::dump_image()
{
  printf("Pupil::draw dumping '%s'\n", mImageName.Data());

  FILE* img = fopen(mImageName.Data(), "w");
  if(img == 0) {
    printf("Pupil::draw can't open screenshot file '%s'.\n", mImageName.Data());
    return;
  }

  tga_header header(w(), h());
  header.dump(img);
  
  unsigned char* xx = new unsigned char[w()*h()*3];
  glReadBuffer(GL_BACK);
  glPixelStorei(GL_PACK_ALIGNMENT,1); 
  glReadPixels(0, 0, w(), h(), GL_BGR, GL_UNSIGNED_BYTE, xx);
  fwrite(xx, 3, w()*h(), img);
  delete [] xx;

  fclose(img);
}

/**************************************************************************/

Pupil::Pupil(OS::ZGlassImg* infoimg, OS::ZGlassView* zgv, int w, int h) :
  Fl_Gl_Window(w,h), OS::A_View(infoimg),
  mInfo(0), mRoot(zgv)
{
  if(fImg) {
    fImg->CheckInFullView(this);
    mInfo = dynamic_cast<PupilInfo*>(fImg->fGlass);
  }

  end();
  label_window();
  mode(FL_RGB | FL_DOUBLE | FL_DEPTH);
  resizable(this);
  size_range(0, 0, 4096, 4096);

  mDriver = new RnrDriver(zgv->fImg->fEye, "GL");
  mBase = 0;
  mCamera = new Camera;
  if(mInfo->GetCameraBase() == 0) {
    mCamera->RotateLF(1, 2, TMath::Pi()/2);
    mCamera->MoveLF(1, -5);
    mCamera->MoveLF(3, 2);
    mCamera->RotateLF(3, 1, 30*TMath::DegToRad());
    // mCamera->mParent = base;
  }

  // pInfo = new PupilInfo();
  // pInfo->SetStampCallback((znode_stamp_f)PupilStamp, this);

  mTexFont = 0;

  bJustCamera = true;
  bFollowBase = false;
  bFullScreen = false;

  bDumpImage  = false;

  // Hmmph ... the locking doesn't seem to be needed any more.
  /*
  Display* rd = (Display*)(dynamic_cast<TGX11*>(gVirtualX)->GetDisplay());
  XLockDisplay(rd);
  XSync(rd, False);
  XLockDisplay(fl_display);
  XSync(fl_display, False);
  */

  _firstp = true;
  size(mInfo->GetWidth()+1, mInfo->GetHeight()+1);
  show();

  /*
  XSync(fl_display, False);
  XUnlockDisplay(fl_display);
  XSync(rd, False);
  XUnlockDisplay(rd);
  */
}

Pupil::~Pupil() {
  // notify eye ... Shell, that is ... !!!!
  delete mCamera;
  if(fImg) fImg->CheckOutFullView(this);
}

/**************************************************************************/

void Pupil::AbsorbRay(Ray& ray)
{
  using namespace RayNS;

  if( ray.fFID != PupilInfo::FID() )
    return;

  switch(ray.fRQN) { 
  case RQN_user_1: {
    if(ray.fRayBits & Ray::RB_CustomBuffer) {
      mImageName.Streamer(*ray.fCustomBuffer);
      ray.ResetCustomBuffer();
      if(mImageName != "") {
	bDumpImage = true;
      }
    }
    fImg->fEye->BreakManageLoop();
    break;
  }

  case RQN_user_2: {
    size(mInfo->GetWidth(), mInfo->GetHeight());
  }

    // need case for link change (or rebase camera in draw()).
  }
}

/**************************************************************************/
// Magick
/**************************************************************************/

void Pupil::SetProjection1()
{
  glLoadIdentity();
  glViewport(0,0,w(),h());
}

void Pupil::SetProjection2()
{
  gluPerspective(mInfo->GetFOV(), (double)w()/h(),
		 mInfo->GetNearClip(),  mInfo->GetFarClip());
}

void Pupil::SetCameraView()
{
  ZTrans z;
  ZNode* cam_base = mInfo->GetCameraBase();
  if(cam_base) {
    auto_ptr<ZTrans> t( mInfo->ToPupilFrame(cam_base) );
    if(t.get() != 0) {
      z *= *t;
    } else {
      printf("Pupil::SetCameraView cam_base is not connected ... ignoring.\n");
    }
    z *= mCamera->GetTrans();
  } else {
    z = mCamera->GetTrans();
  }

  ZNode* look_at = mInfo->GetLookAt();
  if(look_at) {
    auto_ptr<ZTrans> t( mInfo->ToPupilFrame(look_at) );
    if(t.get() != 0) {
      TVector3 cam_pos(   z(1,4),    z(2,4),    z(3,4));
      TVector3 obj_pos((*t)(1,4), (*t)(2,4), (*t)(3,4));
      TVector3 delta = obj_pos - cam_pos;
      delta.SetMag(1);
      
      TVector3 up(0, 0, 0); // take absolute z for up reference
      bool done = false;
      ZNode* up_ref = mInfo->GetUpReference();
      if(up_ref != 0) {
	auto_ptr<ZTrans> t( mInfo->ToPupilFrame(up_ref) );
	if(t.get() != 0) {
	  Int_t c = mInfo->GetUpRefAxis();
	  up.SetXYZ((*t)(1u,c), (*t)(2u,c), (*t)(3u,c));
	  done = true;
	} else {
	  printf("Pupil::SetCameraView up_ref not connected ... ignoring.\n");
	}
      }
      if(!done) {
	int idx = mInfo->GetUpRefAxis();
	if(idx > 0) up(idx-1) = 1;
      }

      Double_t dp = up.Dot(delta);
      // should have: if(dp < eps) ... but then need pos history
      up -= dp*delta;
      gluLookAt(z(1u,4u), z(2u,4u), z(3u,4u),
		(*t)(1u,4u), (*t)(2u,4u), (*t)(3u,4u),
		up.x(), up.y(), up.z());

      return;
    } else {
      printf("Pupil::SetCameraView look_at is not connected ... ignoring.\n");
    }
  }
  gluLookAt(z(1,4), z(2,4), z(3,4),
	    z(1,4)+z(1,1), z(2,4)+z(2,1), z(3,4)+z(3,1),
	    z(1,3), z(2,3), z(3,3));
}

/**************************************************************************/

/*
void Pupil::Rebase(ZNode* newbase, bool keeppos)
{
  if(!newbase) return;
  if(bFollowBase) { 
    if(keeppos) {
      ZTrans* t = ZNode::BtoA(newbase, mBase);
      *t *= mCamera->RefTrans();
      mCamera->RefTrans() = *t;
      delete t;
    }
    mCamera->mParent = newbase;
  }
  mBase = newbase; Label();
}
*/

void Pupil::XtachCamera()
{
  // Transforms camera Base<-> MotherCS based on state of bFollowBase.
  // Does not check current affiliation of mCamera ... probably should.

  /*
  bFollowBase = !bFollowBase; 
  ZTrans* t;
  if(bFollowBase) {
    t = ZNode::BtoA(mBase, mForestView->GetMother());
    mCamera->mParent = mBase;
  } else {
    t = ZNode::BtoA(mForestView->GetMother(), mBase);
    mCamera->mParent = mForestView->GetMother();
  }
  *t *= *mCamera;
  (ZTrans&)(*mCamera) = *t;
  delete t;
  */
}

void Pupil::JumpCameraAt(ZGlass* lens)
{
  // Warps camera towards lens.
  // Case when if camera is attached to some lens IS NOT coded !!!

  ZNode* node = dynamic_cast<ZNode*>(lens);
  if(node == 0) return;
  auto_ptr<ZTrans> t( mInfo->ToPupilFrame(node) );
  if(t.get() == 0) return;

  TVector3 x = t->GetPosVec3() - mCamera->RefTrans().GetBaseVec3(4);
  Double_t dist = x.Mag();
  x = x.Unit();
  TVector3 y = mCamera->RefTrans().GetBaseVec3(2);
  y -= (y.Dot(x))*x;
    
  // Now reuse t to hold new camera transformation.
  t->SetBaseVec3(1, x);
  t->SetBaseVec3(2, y);
  t->SetBaseVec3(3, x.Cross(y));
  t->SetBaseVec3(4, mCamera->RefTrans().GetBaseVec3(4));
  t->MoveLF(1, dist - mInfo->GetMSMoveFac() * 
	    TMath::Power(10, mInfo->GetMoveOM() + 2) );
  mCamera->SetTrans(*t);

  // cout << "CameraTrans:\n" << *t;

  redraw();
}

/**************************************************************************/

void Pupil::FullScreen()
{
  if(bFullScreen) {
    fullscreen_off(mFSx, mFSy, mFSw, mFSh);
  } else {
    mFSx = x(); mFSy = y(); mFSw = w(); mFSh = h();
    fullscreen();
  }
  bFullScreen = !bFullScreen;
}

/**************************************************************************/
// Render & Pick
/**************************************************************************/

void Pupil::Render()
{
  // Calls rnr driver to perform actual rendering.
  // Used by draw() and Pick().

  mDriver->SetMaxDepth(mInfo->GetMaxDepth());
  mDriver->InitLamps();
  mDriver->Render(mRoot);
  for(int l=0; l<mDriver->GetMaxLamps(); ++l) {
    if(mDriver->GetLamps()[l] != 0) {
      // cout <<"Pupil cleaning-up a dirty lamp ...\n";
      (mDriver->GetLamps()[l])->CleanUp(mDriver);
    }
  }
}

/**************************************************************************/
// Picking
/**************************************************************************/

namespace {
  struct pick_menu_data : public FTW_Shell::mir_call_data_img {
    Pupil* pupil;

    pick_menu_data(Pupil* p, OS::ZGlassImg* i) :
      mir_call_data_img(i, 0, 0), pupil(p) {}
  };

  void jump_to_cb(Fl_Widget* w, pick_menu_data* ud)
  {
    ud->pupil->JumpCameraAt(ud->img->fGlass);
  }

  void fill_pick_menu(Pupil* pup, OS::ZGlassImg* img, Fl_Menu_Button& menu,
		      FTW_Shell::mir_call_data_list& mcdl, const string& prefix)
  {
    mcdl.push_back(new pick_menu_data(pup, img));

    menu.add(GForm("%sJumpTo", prefix.c_str()),
	     0, (Fl_Callback*)jump_to_cb, mcdl.back(), FL_MENU_DIVIDER);
 
  }
}

void Pupil::Pick()
{
  // RedBook snatch
  GLsizei bs = mInfo->GetBuffSize();
  GLuint* b = new GLuint[bs];
  glSelectBuffer(bs, b);
  glRenderMode(GL_SELECT);
  glInitNames();

  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix((GLdouble) mMouseX, (GLdouble)(vp[3] - mMouseY),
		mInfo->GetPickR(), mInfo->GetPickR(), vp);
  SetProjection2();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetCameraView();

  Render();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  GLint n = glRenderMode(GL_RENDER);

  if (n > 0) {
    FTW_Shell* shell = mRoot->fImg->fEye->GetShell();

    Fl_Menu_Button menu(Fl::event_x_root(), Fl::event_y_root(), 0, 0, 0);
    menu.textsize(shell->cell_fontsize());

    FTW_Shell::mir_call_data_list mcdl;
    list<glass_data> gdl;

    GLuint* x = b;
    // create a list of picked lenses sorted by z buffer
    for(int i=0; i<n; i++) {
      GLuint m = *x; x++;

      if(x - b + 2 + m > mInfo->GetBuffSize()) {
        cout <<"Pupil::Pick overflow of selection buffer, ignoring that entry\n";
        continue;
      }

      float zmin = (float) *x/0x7fffffff;
      x++;
      // float zmax = (float) *x/0x7fffffff;
      x++;

      UInt_t id = x[m-1];

      OS::ZGlassImg* root_img = mRoot->fImg->fEye->DemangleID(id);
      if(!root_img) {
        printf("Pupil::Pick root_img null for id=%d.\n", id);
        continue;
      }

      ZGlass* lens = root_img->fGlass;
      // if(!lens) continue; // MT: this should NEVER happen.
     
      glass_data gd(root_img, zmin, lens->GetName());;
      
      // fill gd.parents list with contents of the pick record
      for(int j=m-2; j>=0; --j) {
        UInt_t p_id = x[j];
        OS::ZGlassImg* img = mRoot->fImg->fEye->DemangleID(p_id);
        if(!img) {
          printf("Pupil::Pick parent img null for id=%d.\n", p_id);
          continue;
        }
        // ZGlass* parent = img->fGlass;
        // if(!parent) continue; // MT: this should NEVER happen.
        gd.parents.push_back(img);
      }
      
      list<glass_data>::iterator ins_pos = gdl.begin();
      while(ins_pos != gdl.end() && ins_pos->z < gd.z) {
        ++ins_pos;
      }
      gdl.insert(ins_pos, gd);  
      x += m;
    }

    // create menu entries
    int loc = 1;
    for( list<glass_data>::iterator gdi = gdl.begin(); gdi!=gdl.end(); ++gdi) {
      if(mInfo->GetPickDisp() != 0) {    
        Float_t near = mInfo->GetNearClip();
        Float_t far  = mInfo->GetFarClip();
        Float_t zdist = near*far/(far - gdi->z/2*(far - near));
        if(mInfo->GetPickDisp() == 1)
          gdi->name = GForm("%2d. (%6.3f)  %s/",  loc, zdist, gdi->name.c_str()); 
        else
          gdi->name = GForm("%2d. (%6.3f%%)  %s/", loc, 100*(zdist/far), gdi->name.c_str()); 
      } else {
        gdi->name = GForm("%2d. %s/", loc, gdi->name.c_str());
      }
      ++loc;

      shell->FillImageMenu(gdi->img, menu, mcdl, gdi->name);
      fill_pick_menu(this, gdi->img, menu, mcdl, gdi->name);

      // iterate through the list of parents
      for(OS::lpZGlassImg_i pi=gdi->parents.begin(); pi!=gdi->parents.end(); ++pi) {
	string entry(GForm("%sParents/%s/", gdi->name.c_str(), (*pi)->fGlass->GetName()));
	shell->FillImageMenu(*pi, menu, mcdl, entry);
	fill_pick_menu(this, *pi, menu, mcdl, entry);
      }
    }

    menu.popup();

  } // if(n>0)

  delete [] b;
}

/**************************************************************************/
// FLTK virtuals
/**************************************************************************/

void Pupil::draw()
{
  // The following kludge somewhat cures the problem with
  // non present default font!
  if(_firstp) {
    size(mInfo->GetWidth(), mInfo->GetHeight());
    _firstp = false;
    // printf("%d,%d : %d,%d\n", x(), y(), w(), h());
    return;
  }

  GTime start_time(GTime::I_Now);

  // if (!valid()) {
  glMatrixMode(GL_PROJECTION);
  SetProjection1();
  SetProjection2();
  // }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetCameraView();

  { // clear
    const ZColor c = mInfo->GetClearColor();
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

  static const GLfloat mat_diffuse[]   = { 1.0, 1.0, 1.0, 1.0 };
  static const GLfloat mat_specular[]  = { 0.2, 0.2, 0.2, 1.0 };
  static const GLfloat mat_shininess[] = { 60.0 };

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, mInfo->GetLiMo2Side());
  glPolygonMode(GL_FRONT, (GLenum)mInfo->GetFrontMode());
  glPolygonMode(GL_BACK,  (GLenum)mInfo->GetBackMode());

  if(mInfo->GetBlend()) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  // This is stoopid. Need a Stone to encapsulate this. Or a Glass.
  // See ZRlRnrNameCtrl. Should have a stack of those in RnrDriver, or sth.
  mDriver->SetTextSize(mInfo->GetTextSize());
  mDriver->SetRnrNames(mInfo->GetRnrNames());
  mDriver->SetRnrTiles(mInfo->GetRnrTiles());
  mDriver->SetRnrFrames(mInfo->GetRnrTiles()); // !!!
  mDriver->SetNameOffset(mInfo->GetNameOffset());
  mDriver->RefTextCol() = mInfo->GetTextCol();
  mDriver->RefTileCol() = mInfo->GetTileCol();
  mDriver->SetTilePos("tl");

  if(mTexFont == 0) {
    const char* font = "fontdefault.txf";
    const char* file = GForm("%s/lib/%s", gSystem->Getenv("GLEDSYS"), font);
    mTexFont = GLTextNS::txfLoadFont(file);
    if(mTexFont != 0) {
      GLTextNS::txfEstablishTexture(mTexFont, 0, GL_TRUE);
    } else {
      fprintf(stderr, "Problem loading font %s from file %s ; error: %s.\n",
              font, file, GLTextNS::txfErrorString());
    }
  }
  if(mTexFont) {
    mDriver->fTexFont = mTexFont;
  }

  mDriver->SetWidth(w());
  mDriver->SetHeight(h());

  Render();

  if(mDriver->SizePM() > 0) {
    printf("Pupil::draw position stack not empty (%d).\n", mDriver->SizePM());
    mDriver->ClearPM();
  }

  GLenum gl_err = glGetError();
  if(gl_err) {
    printf("Pupil::draw GL error: %s.\n", gluErrorString(gl_err));
  }

  GTime stop_time(GTime::I_Now);
  GTime rnr_time = stop_time - start_time;

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
  glScalef(1, (float)w()/h(), 1);

  GLfloat ch_size = mInfo->GetCHSize();
  glLineWidth(1);
  if(bJustCamera)	glColor3f(1,0,0);
  else
    if(bFollowBase)	glColor3f(0.8,0.8,0.8);
    else		glColor3f(0.4,0.4,0.4);
  glBegin(GL_LINES); {
    glVertex2f(ch_size, 0);	glVertex2f(ch_size/3,0);
    glVertex2f(-ch_size/3, 0);	glVertex2f(-ch_size,0);
    glVertex2f(0, ch_size);	glVertex2f(0,ch_size/3);
    glVertex2f(0, -ch_size/3);	glVertex2f(0,-ch_size);
  } glEnd();


  if(mInfo->GetShowRPS() == true && mDriver->fTexFont) {
    const string text( GForm("%.1frps", 1/rnr_time.ToDouble() <? 999.9) );
    GLTextNS::RnrTextAt(mDriver, text, 2, 0, 1e-3, mInfo->PtrTextCol(), 0);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  if(bDumpImage) {
    bDumpImage = false;
    dump_image();
  }

}

/**************************************************************************/

namespace {
  const Float_t defMSMoveFac = 200;
  const Float_t defMSRotFac  = 600;
}

int Pupil::handle(int ev)
{
  if(ev == FL_SHORTCUT && Fl::event_key() == FL_Escape && parent() == 0) {
    iconize();
    return 1;
  }

  int x = Fl::event_x(); int y = Fl::event_y();
  //printf("PupilEvent %d (%d, %d)\n", ev, x, y);

  switch(ev) {

  case FL_PUSH: {
    mMouseX = x; mMouseY = y; // reset the drag location
    if(Fl::event_button() == 3) {
      Pick();
    }
    return 1;
  }
  case FL_DRAG: {
    // ZNode* target = mCamera;
    /*
      if(bJustCamera) {
      target = mCamera;
      } else {
      target = mMir[0] ? mForestView->GetSelected(mMir[0]) : mBase;
      if(target == 0) target = mCamera;
      }

      // Beta check
      Text_t *move_cmd, *rotate_cmd;
      UInt_t beta = 0;
      if(Fl::event_state(FL_SHIFT)) {
      move_cmd = "Move"; rotate_cmd = "Rotate";
      ZNode* n = mForestView->GetSelected(mMir[1]); // returns 0 if empty or multiple
      if(n) beta = n->GetSunID();
      } else {
      move_cmd = "MoveLF"; rotate_cmd = "RotateLF";
      }
    */

    bool chg = 0;
    int dx = x - mMouseX, dy = y - mMouseY;
    // Invert dx/dy from "screen" to "camera" coordinates.
    dx = -dx; dy = -dy;
    mMouseX = x; mMouseY = y;
    Float_t move_fac = mInfo->GetMSMoveFac() *
                       TMath::Power(10, mInfo->GetMoveOM());
    Float_t rot_fac  = mInfo->GetMSRotFac() * TMath::TwoPi() / 1000;
    if(Fl::event_state(FL_BUTTON1)) { 
      if(!Fl::event_state(FL_CTRL)) {
	mCamera->MoveLF(1, dy*move_fac*TMath::Power(abs(dy), mInfo->GetAccelExp()));
	//sprintf(x, "1,%g", dy*abs(dy)/mInfo->GetMSMoveFac());
	//mEye->Send(move_cmd, x, target, beta);
      } else {
	mCamera->MoveLF(3, dy*move_fac*TMath::Power(abs(dy), mInfo->GetAccelExp()));
	//sprintf(x, "2,%g", dy*abs(dy)/mInfo->GetMSMoveFac());
	//mEye->Send(move_cmd, x, target, beta);      
      }
      mCamera->MoveLF(2, dx*move_fac*TMath::Power(abs(dx), mInfo->GetAccelExp()));

      //sprintf(x, "3,%g", -dx*abs(dx)/mInfo->GetMSMoveFac());
      //mEye->Send(move_cmd, x, target, beta);
      chg = 1;
    }

    /*
    // Kbd Moves
    if(Fl::event_key(' ')) { mEye->Send(move_cmd, "2, 0.1", target, beta); chg=1; }
    if(Fl::event_key('c')) { mEye->Send(move_cmd, "2, -0.1", target, beta); chg=1; }
    if(Fl::event_key('x')) { mEye->Send(move_cmd, "3, 0.1", target, beta); chg=1; }
    if(Fl::event_key('z')) { mEye->Send(move_cmd, "3, -0.1", target, beta); chg=1; }

    if(Fl::event_key('a')) { mEye->Send(rotate_cmd, "2,3,0.1", target, beta); chg=1; }
    if(Fl::event_key('s')) { mEye->Send(rotate_cmd, "2, 3, -0.1", target, beta); chg=1; }
    */

    // cout << mMouseX << " " << mMouseY << " " << dx << " " << dy << endl;
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
	//sprintf(foo,"3,1,%8g", dx/mInfo->GetMSRotFac());
      } else {
	mCamera->RotateLF(2, 3, -dx*rot_fac);
	//sprintf(foo,"2,3,%8g", -dx/mInfo->GetMSRotFac());
      }
      //mEye->Send(rotate_cmd, foo, target, beta);
      chg=1;
    }
    if(chg) redraw();
  }
  case FL_KEYBOARD: {
    switch(Fl::event_key()) {

    case FL_Home:
      mCamera->Home(); redraw();
      return 1;

    case 'f':
      FullScreen(); redraw();
      return 1;

    case FL_F+1:
      mRoot->fImg->fEye->GetShell()->SpawnMTW_View(mRoot->fImg);
      return 1;

      /*
	case FL_Tab:
	bJustCamera = !bJustCamera; redraw();
	return 1;
	case 'q':
	XtachCamera(); redraw();
	return 1;

	case FL_F+2:
	return 1;
	case FL_F+3:
	(Int_t&)(mMir[0]) +=1;
	if(mMir[0] == ForestView::SID_name)
	mMir[0] = ForestView::SID_current;
	redraw(); return 1;
	case FL_F+4:
	(Int_t&)(mMir[1]) += 1;
	if(mMir[1] == ForestView::SID_name)
	mMir[1] = ForestView::SID_current;
	redraw(); return 1;
      
	case FL_Left: // previous
	Rebase(mBase->PrevBro(), !Fl::event_state(FL_SHIFT));
	redraw(); return 1;
	case FL_Right: // next
	Rebase(mBase->NextBro(), !Fl::event_state(FL_SHIFT));
	redraw(); return 1;
	case FL_Up: // parent
	Rebase(mBase->Parent(), !Fl::event_state(FL_SHIFT));
	redraw(); return 1;
	case FL_Down: // first son
	Rebase(mBase->Glasses().First(), !Fl::event_state(FL_SHIFT));
	redraw(); return 1;
      */

    } // switch(Fl::event_key())

    return 1;
  } // case FL_KEYBOARD

  } // switch(ev)
  
  return 0;
}

/*
void Pupil::draw_overlay()
{
  // Safr ... this works not; redraw_overlay commented out in ctor.

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  SetProjection1();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);

  gl_color(FL_GREEN);
  glBegin(GL_LINES); {
    glVertex3f(-0.5, -5, -5);	glVertex3f(0.5, 5, 5);
  } glEnd();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}
*/

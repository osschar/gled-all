// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Pupil.h"
#include "Eye.h"
#include "FTW_Leaf.h"
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

/**************************************************************************/

void PupilStamp(ZNode* n, Pupil* p) { p->valid(0); p->redraw(); }

void Pupil::Label()
{
  snprintf(mLabel, 127, "Pupil [%s]", mBase->GetName());
  label(mLabel);
  redraw();
}

Pupil::Pupil(PupilInfo* info, OptoStructs::ZGlassView* zgv, // FTW_Leaf* leaf,
	     int w, int h) :
  Fl_Gl_Window(w,h), mInfo(info), mRoot(zgv) // mLeaf(leaf)
{
  end();
  label(GForm("pupil: %s; %s", mInfo->GetName(), mInfo->GetTitle()));

  mBase = 0;
  mCamera = new Camera;
  // mCamera->mParent = base;
  mCamera->MoveLF(1, -5);
  // Label();

  // pInfo = new PupilInfo();
  // pInfo->SetStampCallback((znode_stamp_f)PupilStamp, this);

  resizable(this);
  mode(FL_RGB | FL_DOUBLE | FL_DEPTH);
  bJustCamera = true;
  bFollowBase = false;

  Display* rd = (Display*)(dynamic_cast<TGX11*>(gVirtualX)->GetDisplay());
  XLockDisplay(rd);
  XSync(rd, False);
  XLockDisplay(fl_display);
  XSync(fl_display, False);

  show();
  //redraw_overlay();

  XSync(fl_display, False);
  XUnlockDisplay(fl_display);
  XSync(rd, False);
  XUnlockDisplay(rd);

  mDriver = new RnrDriver(zgv->fImg->fEye, "GL");
}

Pupil::~Pupil() {
  // notify eye ... Shell, that is ... !!!!
  delete mCamera;
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
      TVector3 cam_pos(z(1u,4u), z(2u,4u), z(3u,4u));
      TVector3 obj_pos((*t)(1u,4u), (*t)(2u,4u), (*t)(3u,4u));
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
  gluLookAt(z(1u,4u), z(2u,4u), z(3u,4u),
	    z(1u,4u)+z(1u,1u), z(2u,4u)+z(2u,1u), z(3u,4u)+z(3u,1u),
	    z(1u,2u), z(2u,2u), z(3u,2u));
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

void Pupil::fltk_pick_callback(Fl_Widget* w, pick_data* pd)
{
  pd->pupil->pick_callback(w, pd);
}

void Pupil::pick_callback(Fl_Widget* w, pick_data* pd)
{
  switch(pd->operation) {
  case p_open_view: {
    Eye* eye = GetRoot()->fImg->fEye;
    eye->GetShell()->SpawnMTW_View(eye->DemanglePtr(pd->lens));
    break;
  }
  case p_null:
  default:
    break;
  }
}

namespace {

  void make_menu(Fl_Menu_Button& menu, list<Pupil::pick_data>& pdl,
		 const string& prefix,
		 Pupil* pupil, ZGlass* lens)
  {
    // Make a menu for lens. Called from Pupil::Pick().
    // Presently rather empty and non-configurable.

    pdl.push_back( Pupil::pick_data(pupil, lens, Pupil::p_open_view) );
    menu.add(GForm("%s/Open view ...", prefix.c_str()), 0,
	     (Fl_Callback*)Pupil::fltk_pick_callback, &pdl.back(), FL_MENU_DIVIDER);
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
		mInfo->GetPickW(), mInfo->GetPickH(), vp);
  SetProjection2();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetCameraView();

  Render();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  GLint n = glRenderMode(GL_RENDER);

  if(n>0) {
    Fl_Menu_Button menu(Fl::event_x_root(), Fl::event_y_root(), 0, 0, 0);
    menu.textsize(mRoot->fImg->fEye->GetShell()->cell_fontsize());

    GLuint* x = b;
    list<pick_data> pdl;
    for(int i=0; i<n; i++) {
      GLuint m = *x; x++;

      if(x - b + 2 + m > mInfo->GetBuffSize()) {
	cout <<"Pupil::Pick overflow of selection buffer, ignoring that entry\n";
	continue;
      }
 
      float zmin = (float) *x/0x7fffffff; x++;
      float zmax = (float) *x/0x7fffffff; x++;

      // i know there is a single entry (see ZGlass Pre/PostDraw ... glPush/PopName)
      // `for' is just in case some joker fiddles the ZNode
      // This joker has just been me ...

      UInt_t id = x[m-1];
      ZGlass* glass = mRoot->fImg->fEye->GetSaturn()->DemangleID(id);
      if(!glass) continue;
      const char* name = glass->GetName();

      pdl.push_back( pick_data(this, glass) );
      menu.add(name, 0, (Fl_Callback*)fltk_pick_callback, &pdl.back(), FL_SUBMENU);
      make_menu(menu, pdl, string(name), this, glass);
      for(int j=m-2; j>=0; --j) {
	UInt_t p_id = x[j];
	ZGlass* parent = mRoot->fImg->fEye->GetSaturn()->DemangleID(p_id);
	if(!parent) continue;
	make_menu(menu, pdl,
		  string(GForm("%s/Parents/%s", name, parent->GetName())),
		  this, parent);
      }

      x += m;
    }
    const Fl_Menu_Item* mi = menu.popup();
    // pdl going out of scope
  } // if(n>0)
  delete [] b;
}

/**************************************************************************/
// FLTK virtuals
/**************************************************************************/

void Pupil::draw()
{
  if(mDriver->fTexFont == 0) {
    GLTextNS::TexFont *txf;
    const char* font = "fontdefault.txf";
    const char* file = GForm("%s/lib/%s", gSystem->Getenv("GLEDSYS"), font);
    txf = GLTextNS::txfLoadFont(file);
    if(txf != 0) {
      GLTextNS::txfEstablishTexture(txf, 0, GL_TRUE);
      mDriver->fTexFont = txf;
    } else {
      fprintf(stderr, "Problem loading font %s from file %s ; error: %s.\n",
              font, file, GLTextNS::txfErrorString());
    }
  }

  GTime start_time(GTime::I_Now);

  //if (!valid()) {
  glMatrixMode(GL_PROJECTION);
  SetProjection1();
  SetProjection2();
  //valid(1);
  //}

  { // clear
    const ZColor c = mInfo->GetClearColor();
    glClearColor(c.r(), c.g(), c.b(), c.a());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetCameraView();

  // Should move this shit somewhere ... and hope other shit cleans up ... naaah
  // i think i would better leave it here a bit longer
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  // This is overriden with first light ... just in case there are no lights in the scene
  GLfloat ambient[] = { 0.1, 0.1, 0.1, 1.0 };
  GLfloat position[] = { 0, 0.0, 10.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glEnable(GL_LIGHT0);

  GLfloat mat_diffuse[] = { 0.5, 0.2, 0.6, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 200.0 };

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, mInfo->GetLiMo2Side());
  glPolygonMode(GL_FRONT, (GLenum)mInfo->GetFrontMode());
  glPolygonMode(GL_BACK, (GLenum)mInfo->GetBackMode());

  if(mInfo->GetBlend()) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  /*
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH, GL_FASTEST);
    glHint(GL_LINE_SMOOTH, GL_FASTEST);
  */

  Render();

  GLenum gl_err = glGetError();
  if(gl_err) {
    printf("GL error: %s\n", gluErrorString(gl_err));
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
  //SetProjection1();
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glScalef(1, (float)w()/h(), 1);

  GLfloat ch_size = mInfo->GetCHSize();
  glLineWidth(2);
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

  /*
    glPointSize(4);
    if(mMir[0] > 0) {
    Float_t col[3] = {0,0,0}; col[ mMir[0]-1 ] = 1; glColor3fv(col);
    glBegin(GL_POINTS); glVertex2f(-ch_size, ch_size); glEnd();
    }
    if(mMir[1] > 0) {
    Float_t col[3] = {0,0,0}; col[ mMir[1]-1 ] = 1; glColor3fv(col);
    glBegin(GL_POINTS); glVertex2f(ch_size, ch_size); glEnd();
    }
  */

  if(mInfo->GetShowRPS() == true && mDriver->fTexFont) {
    GLTextNS::TexFont *txf = mDriver->fTexFont;

    int step = txf->max_ascent + txf->max_descent;

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT);

    glLoadIdentity();
    glTranslatef(-1, -1, 0.0);
    glScalef(2.0/w(), 2.0/h(), 1);
    glTranslatef(0, h() - txf->max_ascent, 0);
    glColor3f(0,1,1);

    GLTextNS::txfBindFontTexture(txf);
  
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT, GL_FILL);

    // int width, ascent, descent;
    // GLTextNS::txfGetStringMetrics(txf, text, len, &width, &ascent, &descent);
    const char* text = GForm("rps %.1f", 1/rnr_time.ToDouble() <? 999.9);
    GLTextNS::txfRenderString(txf, text, strlen(text));

    glPopAttrib();
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  //glFinish();
  //glFlush();
  //glXWaitGL();
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
    ZNode* target = mCamera;
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
    mMouseX = x; mMouseY = y;
    // Fwd/Bck
    if(Fl::event_state(FL_BUTTON1)) { 
      char x[64];
      if(!Fl::event_state(FL_CTRL)) {
	mCamera->MoveLF(1, dy*abs(dy)/mInfo->GetMSMoveFac());
	//sprintf(x, "1,%g", dy*abs(dy)/mInfo->GetMSMoveFac());
	//mEye->Send(move_cmd, x, target, beta);
      } else {
	mCamera->MoveLF(2, dy*abs(dy)/mInfo->GetMSMoveFac());
	//sprintf(x, "2,%g", dy*abs(dy)/mInfo->GetMSMoveFac());
	//mEye->Send(move_cmd, x, target, beta);      
      }
      mCamera->MoveLF(3, -dx*abs(dx)/mInfo->GetMSMoveFac());
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
      char foo[16];
      if(!Fl::event_state(FL_CTRL)) {
	mCamera->RotateLF(1,2,dy/mInfo->GetMSRotFac());
      } else {
	mCamera->RotateLF(1,2,-dy/mInfo->GetMSRotFac());
      }
      chg=1;
    }
    if(Fl::event_state(FL_BUTTON2) && dx!=0) {
      char foo[16];
      if(!Fl::event_state(FL_CTRL)) {
	mCamera->RotateLF(3,1,dx/mInfo->GetMSRotFac());
	//sprintf(foo,"3,1,%8g", dx/mInfo->GetMSRotFac());
      } else {
	mCamera->RotateLF(2,3,-dx/mInfo->GetMSRotFac());
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

      /*
	case FL_Tab:
	bJustCamera = !bJustCamera; redraw();
	return 1;
	case 'q':
	XtachCamera(); redraw();
	return 1;
	case FL_F+1:
	//if(!pInfoView) pInfoView = new ZNodeMixer(mInfo);
	//pInfoView->show();
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

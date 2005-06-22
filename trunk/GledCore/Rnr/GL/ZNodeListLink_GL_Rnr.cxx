// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNodeListLink_GL_Rnr.h"

#include <RnrBase/RnrDriver.h>
#include <RnrBase/Fl_Event_Enums.h>
#include <Rnr/GL/SphereTrings.h>
#include <GledView/GLTextNS.h>

#include <Stones/ZMIR.h>
#include <Eye/Eye.h>

#include <GL/gl.h>

/**************************************************************************/

void ZNodeListLink_GL_Rnr::_init()
{}

/**************************************************************************/

//void ZNodeListLink_GL_Rnr::PreDraw(RnrDriver* rd)
//{}

#define ZNLL ZNodeListLink

void ZNodeListLink_GL_Rnr::Draw(RnrDriver* rd)
{
  ZNodeListLink& M = *mZNodeListLink;
  if(M.mContents == 0 || M.mContents->IsEmpty())
    return;

  obtain_rnrmod(rd, mFontRMS);

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  if(M.mViewMode != ZNLL::VM_Nop) {
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(rd->GetProjBase());
    switch(M.mViewMode) {
    case ZNLL::VM_OrthoPixel:
      glOrtho(0, rd->GetWidth(), 0, rd->GetHeight(), M.mOrthoNear, M.mOrthoFar);
      break;
    case ZNLL::VM_OrthoFixed:
      glOrtho(0, M.mOrthoW, 0, M.mOrthoH, M.mOrthoNear, M.mOrthoFar);
      break;
    default:
      break;
    }
    glMatrixMode(GL_MODELVIEW);
  }

  lpZGlass_t cont; M.mContents->Copy(cont);

  Int_t nx=0, ny=0, nz=0;
  Int_t *ls[3], *ns[3];
  switch(M.mStepMode) { 
  case ZNLL::SM_XYZ:
    ls[0] = &M.mNx; ls[1] = &M.mNy; ls[2] = &M.mNz;
    ns[0] = &nx;    ns[1] = &ny;    ns[2] = &nz;
    break;
  default:
    printf("Unsupported step-mode, assuming YXZ\n");
  case ZNLL::SM_YXZ:
    ls[0] = &M.mNy; ls[1] = &M.mNx; ls[2] = &M.mNz;
    ns[0] = &ny;    ns[1] = &nx;    ns[2] = &nz;
    break;
  }
  
  if(M.mViewMode == ZNLL::VM_OrthoPixel) {
    Float_t x = M.mOx; if(x < 0) x += rd->GetWidth();
    Float_t y = M.mOy; if(y < 0) y += rd->GetHeight();
    glTranslatef(x, y, M.mOz);
  } else {
    glTranslatef(M.mOx, M.mOy, M.mOz);
  }

  for(lpZGlass_i i=cont.begin(); i!=cont.end(); ++i) {

    string lens_name((*i)->GetName());

    ZColor back_color = M.mBoxColor;
    // if(*i selected)
    //  back_color = M.mBoxColor();
    if(*i == M.mCurrent)
      back_color += M.mCurColMod;

    glPushMatrix();
    glTranslatef(nx*M.mDx, ny*M.mDy, nz*M.mDz);
    glPushName((*i)->GetSaturnID());

    if(M.bDrawBox) {

      // !!!! Need special sub-if for ortho-mode, use Sz as border width

      glColor4fv(back_color());
      glPushMatrix();
      glTranslatef(M.mBoxDx, M.mBoxDy, M.mBoxDz);
      glScalef(M.mBoxSx, M.mBoxSy, M.mBoxSz);
      SphereTrings::UnitBox();
      glPopMatrix();
    }

    if(M.bDrawText) {
      GLTextNS::TexFont *txf = ((ZRlFont_GL_Rnr*)mFontRMS.rnr())->GetFont();

      int width, ascent, descent;
      GLTextNS::txfGetStringMetrics(txf, lens_name.c_str(), lens_name.length(),
				    width, ascent, descent);
      ascent  = txf->max_ascent;
      descent = txf->max_descent;

      int   h_box = ascent + descent;
      float scale = M.mTextYSize / h_box;
      float scaled_w = width*scale;
      if(scaled_w > M.mTextMaxLen)
	scaled_w = M.mTextMaxLen;

      glPushMatrix();
      glTranslatef(M.mTextDx, M.mTextDy, M.mTextDz);
      glRotatef( M.mTextA1, 0, 0, 1);
      glRotatef(-M.mTextA2, 0, 1, 0);
      glRotatef( M.mTextA3, 1, 0, 0);
      if(M.bTextCenter) {
	glTranslatef(-scaled_w/2, 0, 0);
      }
      glNormal3f(0, 0, 1);

      if(M.bDrawTile || M.bDrawFrame) {
	float x0 = -M.mXBorder;
	float x1 = (M.bFullTile ? M.mTextMaxLen : scaled_w) + M.mXBorder;
	float y0 = -M.mYBorder  - descent*scale;
	float y1 =  M.mYBorder  + ascent*scale;

	if(M.bDrawTile) {
	  glEnable(GL_POLYGON_OFFSET_FILL);
	  glPolygonOffset(1, 1);
	  glColor4fv(back_color());
	  glBegin(GL_QUADS);
	  glVertex2f(x0, y0); glVertex2f(x1, y0);
	  glVertex2f(x1, y1); glVertex2f(x0, y1);
	  glEnd();
	  glDisable(GL_POLYGON_OFFSET_FILL);
	}
	if(M.bDrawFrame) {
	  glColor4fv(M.mTextColor());
	  glBegin(GL_LINE_LOOP);
	  glVertex2f(x0, y0); glVertex2f(x1, y0);
	  glVertex2f(x1, y1); glVertex2f(x0, y1);
	  glEnd();
	}
      }

      glScalef(scale, scale, 1);
      glColor4fv(M.mTextColor());
      GL_Capability_Switch texure_on(GL_TEXTURE_2D, true);
      GLTextNS::txfBindFontTexture(txf);
      txfRenderString(txf, lens_name.c_str(), lens_name.length(),
		      M.mTextMaxLen/scale, M.mTextFadeW);
      glPopMatrix();
    }

    glPopName();
    glPopMatrix();

    (*ns[0])++;
    if(*ns[0] >= *ls[0]) {
      *ns[0] = 0; (*ns[1])++;
      if(*ns[1] >= *ls[1]) {
	*ns[1] = 0; (*ns[2])++;
	if(*ns[2] >= *ls[2]) {
	  printf("out of space ... proceeding any way\n");
	}
      }
    }
  }

  if(M.mViewMode != ZNLL::VM_Nop) {
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
  }

  glPopAttrib();
}

//void ZNodeListLink_GL_Rnr::PostDraw(RnrDriver* rd)
//{}

/**************************************************************************/

int ZNodeListLink_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  // printf("Handloo: %d %d 0x%x 0x%x %p '%s'\n", ev.fEvent,
  // ev.fKey, ev.fButton, ev.fState, ev.fBelowMouse, 
  // ev.fBelowMouse ? ev.fBelowMouse->fGlass->GetName() : "-");

  ZNodeListLink& M = *mZNodeListLink;

  if(ev.fEvent == FL_ENTER) {
    auto_ptr<ZMIR> mir( M.S_SetCurrent(ev.fBelowMouse->fGlass) );
    fImg->fEye->Send(*mir);
    return 1;
  }
  if(ev.fEvent == FL_LEAVE) {
    auto_ptr<ZMIR> mir( M.S_SetCurrent(0) );
    fImg->fEye->Send(*mir);
    return 1;
  }

  if(ev.fBelowMouse == 0) return 0;

  if(ev.fEvent == FL_PUSH && ev.fButton == FL_LEFT_MOUSE) {
    GledNS::MethodInfo* mi = M.GetCbackMethodInfo();
    if(mi == 0) return 0;
    ZMIR mir(M.mCbackAlpha, ev.fBelowMouse->fGlass);
    mi->ImprintMir(mir);
    mi->FixMirBits(mir, fImg->fEye->GetSaturnInfo());
    fImg->fEye->Send(mir);
  }

  return 0;
}

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GLTesterOne_GL_Rnr.h"
#include <Gled/GTime.h>
#include <GL/gl.h>

/**************************************************************************/

void GLTesterOne_GL_Rnr::_init()
{}

/**************************************************************************/

void GLTesterOne_GL_Rnr::Draw(RnrDriver* rd)
{
  GTime start(GTime::I_Now);

  if(mGLTesterOne->bUseDispList) {
    ZGlass_GL_Rnr::Draw(rd);
  } else {
    Render(rd);
  }

  Double_t time = start.TimeUntilNow().ToDouble();
  mGLTesterOne->register_result(time);
  if(mGLTesterOne->bPrint)
    printf("GLTesterOne rnr time %lf\n", time);
}

/**************************************************************************/

void GLTesterOne_GL_Rnr::Render(RnrDriver* rd)
{
  GLTesterOne& T = *mGLTesterOne;

  if(T.mTestSelection == GLTesterOne::TT_RnrAtom) {

    Float_t x    = 0;
    Float_t step = T.mXMax / (T.mNSteps - 1);

    switch (T.mTMode) {

    case GLTesterOne::TM_Vertex: {

      switch (T.mRAtom) {
      case GLTesterOne::RA_Point:
	glBegin(GL_POINTS);
	for(int i=0; i<T.mNSteps; ++i, x+=step) glVertex2f(x, 0);
	glEnd();
	break;
      case GLTesterOne::RA_Line:
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<T.mNSteps; ++i, x+=step) glVertex2f(x, 0);
	glEnd();
	break;
      case GLTesterOne::RA_Triangle:
	glBegin(GL_TRIANGLES);
	for(int i=0; i<T.mNSteps; ++i, x+=step) {
	  glVertex2f(x, T.mTringV);
	  glVertex2f(x, -T.mTringV);
	  glVertex2f(x + T.mTringU, 0);
	}
	glEnd();
	break;
      }

      break;
    }

    case GLTesterOne::TM_Translate: {

      switch (T.mRAtom) {
      case GLTesterOne::RA_Point:
	for(int i=0; i<T.mNSteps; ++i) {
	  glBegin(GL_POINTS);
	  glVertex2f(0, 0);
	  glEnd();
	  glTranslatef(step, 0, 0);
	}
	break;
      case GLTesterOne::RA_Line:
	for(int i=0; i<T.mNSteps; ++i) {
	  glBegin(GL_LINES);
	  glVertex2f(0, 0); glVertex2f(step, 0);
	  glEnd();
	  glTranslatef(step, 0, 0);
	}
	break;
      case GLTesterOne::RA_Triangle:
	for(int i=0; i<T.mNSteps; ++i) {
	  glBegin(GL_TRIANGLES);
	  glVertex2f(0, T.mTringV);
	  glVertex2f(0, -T.mTringV);
	  glVertex2f(0 + T.mTringU, 0);
	  glEnd();
	  glTranslatef(step, 0, 0);
	}
	break;
      }

      break;
    }

    case GLTesterOne::TM_PushPopMatrix: {

      switch (T.mRAtom) {
      case GLTesterOne::RA_Point:
	for(int i=0; i<T.mNSteps; ++i, x+=step) {
	  glPushMatrix();
	  glTranslatef(x, 0, 0);
	  glBegin(GL_POINTS);
	  glVertex2f(0, 0);
	  glEnd();
	  glPopMatrix();
	}
	break;
      case GLTesterOne::RA_Line:
	for(int i=0; i<T.mNSteps; ++i, x+=step) {
	  glPushMatrix();
	  glTranslatef(x, 0, 0);
	  glBegin(GL_LINES);
	  glVertex2f(0, 0); glVertex2f(step, 0);
	  glEnd();
	  glPopMatrix();
	}
	break;
      case GLTesterOne::RA_Triangle:
	for(int i=0; i<T.mNSteps; ++i, x+=step) {
	  glPushMatrix();
	  glTranslatef(x, 0, 0);
	  glBegin(GL_TRIANGLES);
	  glVertex2f(0, T.mTringV);
	  glVertex2f(0, -T.mTringV);
	  glVertex2f(0 + T.mTringU, 0);
	  glEnd();
	  glPopMatrix();
	}
	break;
      }

      break;
    }

    }

  } // end if TT_RnrAtom

  else if(T.mTestSelection == GLTesterOne::TT_MatrixOps) {

    switch (T.mMatrixOps) {

    case GLTesterOne::MO_GetFloat: {
      GLfloat m[16];
      for(int i=0; i<T.mNSteps; ++i) {
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
      }
      break;
    }

    case GLTesterOne::MO_GetDouble: {
      GLdouble m[16];
      for(int i=0; i<T.mNSteps; ++i) {
	glGetDoublev(GL_MODELVIEW_MATRIX, m);
      }
      break;
    }

    }
  }
}

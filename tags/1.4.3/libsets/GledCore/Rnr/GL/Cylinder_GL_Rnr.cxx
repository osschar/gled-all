// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Cylinder_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

/**************************************************************************/

void Cylinder_GL_Rnr::_init()
{
  mQuadric = gluNewQuadric();
  gluQuadricDrawStyle(mQuadric, GLU_FILL);
  gluQuadricNormals(mQuadric, GLU_SMOOTH);
}

Cylinder_GL_Rnr::~Cylinder_GL_Rnr()
{
  gluDeleteQuadric(mQuadric);
}

/**************************************************************************/

void Cylinder_GL_Rnr::Render(RnrDriver* rd)
{
  Cylinder& C = *mCylinder;
  rd->GL()->Color(C.mColor);
  glPushMatrix();
  switch (C.mOrientation) {
  case Cylinder::O_Z: break;
  case Cylinder::O_Y: glRotatef(90, -1, 0, 0); break;
  case Cylinder::O_X: glRotatef(90,  0, 1, 0); break;
  }
  if(C.mPhiOffset != 0) glRotatef(C.mPhiOffset*360/C.mLodPhi, 0, 0, 1);
  glTranslatef(0,0,-C.mHeight/2);
  // inner cylinder
  if(C.mRInBase != 0 || C.mRInTop != 0) {
    gluQuadricOrientation(mQuadric,GLU_INSIDE);
    gluCylinder(mQuadric, C.mRInBase, C.mRInTop, C.mHeight,
		C.mLodPhi, C.mLodH);
  }
  // outer cylinder
  gluQuadricOrientation(mQuadric,GLU_OUTSIDE);
  gluCylinder(mQuadric, C.mROutBase, C.mROutTop, C.mHeight,
	      C.mLodPhi, C.mLodH);
  // base and top plates
  if(C.bRnrDisks){
    gluQuadricOrientation(mQuadric,GLU_INSIDE);
    gluDisk(mQuadric, C.mRInBase, C.mROutBase,C.mLodPhi,1);
    gluQuadricOrientation(mQuadric,GLU_OUTSIDE);
    glTranslatef(0,0,C.mHeight);
    gluDisk(mQuadric, C.mRInTop,  C.mROutTop, C.mLodPhi,1);
  }
  glPopMatrix();
}


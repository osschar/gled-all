// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Cylinder_GL_Rnr.h"

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

void Cylinder_GL_Rnr::Draw(RnrDriver* rd)
{
  Cylinder& C = *mCylinder;
  glPushAttrib(GL_CURRENT_BIT);
  glColor4fv(C.mColor());
  glPushMatrix();
  glTranslatef(0,0,-C.mHeight/2);
  // inner cylinder
  if(C.mRInBase != 0 || C.mRInTop != 0) {
    gluQuadricOrientation(mQuadric,GLU_INSIDE);
    gluCylinder(mQuadric, C.mRInBase, C.mRInTop, C.mHeight, 
		C.mLodPhi, C.mLodZ);
  }
  // outer cylinder 
  gluQuadricOrientation(mQuadric,GLU_OUTSIDE);
  gluCylinder(mQuadric, C.mROutBase, C.mROutTop, C.mHeight, 
	      C.mLodPhi, C.mLodZ);
  // base and top plates
  if(C.bRnrDisks){
    gluQuadricOrientation(mQuadric,GLU_INSIDE);
    gluDisk(mQuadric, C.mRInBase, C.mROutBase,C.mLodPhi,1);
    gluQuadricOrientation(mQuadric,GLU_OUTSIDE);
    glTranslatef(0,0,C.mHeight);
    gluDisk(mQuadric, C.mRInTop,  C.mROutTop, C.mLodPhi,1);
  }
  glPopMatrix();
  glPopAttrib();
}


// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Box_GL_Rnr.h"

#include <GL/gl.h>

/**************************************************************************/

namespace {
  GLfloat a = 0.5;
  GLfloat V_Cube[] = { a,a,a,	a,-a,a,		a,-a,-a,	a,a,-a,
		       -a,a,a,	-a,-a,a,	-a,-a,-a,	-a,a,-a };
  
  GLubyte I_Cube[] = { 0,1,2,3,	3,2,6,7,	2,1,5,6,	6,5,4,7,
		       7,4,0,3,	0,4,5,1 };

  GLfloat N_Cube[][3] = { {1,0,0}, {0,0,-1}, {0,-1,0},
			  {-1,0,0}, {0,1,0}, {0,0,1} };
}

void Box_GL_Rnr::Draw(RnrDriver* rd)
{
  glPushAttrib(GL_CURRENT_BIT);
  glColor4fv(mBox->mColor());
  glPushMatrix();
  glScalef(mBox->mA, mBox->mB, mBox->mC);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, V_Cube);

  glBegin(GL_QUADS);
  int idx = 0;
  for(int i=0; i<6; ++i) {
    glNormal3fv(N_Cube[i]);
    for(int j=0; j<4; ++j, ++idx) glArrayElement(I_Cube[idx]);
  }
  glEnd();
  glDisableClientState(GL_VERTEX_ARRAY);

  glPopMatrix();
  glPopAttrib();
}

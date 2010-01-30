// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Box_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <GL/glew.h>

/**************************************************************************/

namespace
{
  const GLubyte I_Cube[] = { 0,1,2,3,	3,2,6,7, 2,1,5,6,
			     6,5,4,7, 7,4,0,3, 0,4,5,1 };

  const GLfloat N_Cube[][3] = { { 1,0,0}, {0,0,-1}, {0,-1,0},
				{-1,0,0}, {0,1, 0}, {0, 0,1} };
}

void Box_GL_Rnr::Draw(RnrDriver* rd)
{
  GLfloat a = 0.5f * mBox->mA;
  GLfloat b = 0.5f * mBox->mB;
  GLfloat c = 0.5f * mBox->mC;

  GLfloat V_Cube[][3] = { { a,b,c}, { a,-b,c}, { a,-b,-c}, { a,b,-c},
			  {-a,b,c}, {-a,-b,c}, {-a,-b,-c}, {-a,b,-c} };

  rd->GL()->Color(mBox->mColor);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, V_Cube);

  glBegin(GL_QUADS);
  int idx = 0;
  for (int i=0; i<6; ++i)
  {
    glNormal3fv(N_Cube[i]);
    for (int j=0; j<4; ++j, ++idx)
      glArrayElement(I_Cube[idx]);
  }
  glEnd();

  glDisableClientState(GL_VERTEX_ARRAY);
}

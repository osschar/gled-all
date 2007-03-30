// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SphereTrings.h"
#include <math.h>

// Use normalized normals or thou shalt burn in hell!
// OK ... all normals identical to vertexen ... that's why this is SphereTring

namespace {
  float sqrt2 = sqrt(2);
  float sqrt3 = sqrt(3);
  float x = sqrt(sqrt3/2), z = (1-sqrt3)/2, y = sqrt3*x/2;
  // Tethraedron
  GLfloat V_Tetrus[] = { 0,0,1,	x,0,z,	-x/2,y,z,	-x/2,-y,z };
  //GLfloat N_Tetrus[] = { 0,0,1,	x,0,z,	-x/2,y,z,	-x/2,-y,z };
  GLubyte I_Tetrus[] = { 0,1,2,	0,2,3,	0,3,1,	1,3,2 };

  float Ca = 2/sqrt3;
  float a = Ca/2;
  // Cube ... watch it ... a = 2 sqrt(3)/3
  GLfloat V_Cube[] = { a,a,a,	a,-a,a,		a,-a,-a,   a,a,-a,
		      -a,a,a,  -a,-a,a,        -a,-a,-a,  -a,a,-a
  };
  GLubyte I_Cube[] = { 0,1,2,3,	3,2,6,7,	2,1,5,6,   6,5,4,7,
		       7,4,0,3,	0,4,5,1 
  };
  GLfloat N_Cube[] = { 1,0,0,   0,0,-1,         0,-1,0,   -1,0,0,
		       0,1,0,   0,0,1 
  };

  float b = sqrt2/2;
  GLfloat V_Octus[] = { 0,0,1, b,b,0, -b,b,0, -b,-b,0, b,-b,0, 0,0,-1 };
  GLubyte I_Octus[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,1, 5,2,1, 5,3,2, 5,4,3, 5,1,4 };
}

namespace SphereTrings {
  GLfloat  CubeA	= Ca;
  GLfloat  OctusA	= sqrt2;

  GLfloat *Vertexen[5]	= { V_Tetrus, V_Cube, V_Octus };
  GLfloat *Normaleen[5]	= { V_Tetrus, N_Cube, V_Octus };
  GLubyte *Indexen[5]	= { I_Tetrus, I_Cube, I_Octus };
  GLenum   GLmode[5]	= { GL_TRIANGLES, GL_QUADS, GL_TRIANGLES };
  GLsizei  IndexSize[5]	= { 12, 24, 24 };
  GLsizei  NumFaces[5]  = {  4,  6,  8 };
  GLsizei  NumVerts[5]  = {  4,  8,  6 };
  GLubyte  Vert5Face[5] = {  3,  4,  3 };
}

/**************************************************************************/
// These two set up and execute DrawElements ... you should enable texturing
//	or coloring yourself

using namespace SphereTrings;

void SphereTrings::Render(int i, bool flat_p) {
  if(flat_p) {
    GLfloat *v   = Vertexen[i];
    GLfloat *n   = Normaleen[i];
    GLubyte *idx = Indexen[i];
    int x = NumFaces[i];
    glBegin(GLmode[i]);
    while(x-- > 0) {
      glNormal3fv(n);
      glVertex3fv(&v[*(idx++) * 3]);
      glVertex3fv(&v[*(idx++) * 3]);
      glVertex3fv(&v[*(idx++) * 3]);
      glVertex3fv(&v[*(idx++) * 3]);
      n += 3;
    }
    glEnd();
  } else {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Vertexen[i]);
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, Normaleen[i]);

    glDrawElements(GLmode[i], IndexSize[i], GL_UNSIGNED_BYTE, Indexen[i]);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
  }
}

void SphereTrings::EnableGL(int i)
{
  glEnableClientState(GL_VERTEX_ARRAY); glEnableClientState(GL_NORMAL_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, Vertexen[i]);
  glNormalPointer(GL_FLOAT, 0, Normaleen[i]);
}

void SphereTrings::DrawAndDisableGL(int i)
{
  glDrawElements(GLmode[i], IndexSize[i], GL_UNSIGNED_BYTE, Indexen[i]);
  glDisableClientState(GL_VERTEX_ARRAY); glDisableClientState(GL_NORMAL_ARRAY);
}
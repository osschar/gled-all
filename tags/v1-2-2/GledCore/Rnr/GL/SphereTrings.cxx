// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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
  GLfloat V_Cube[] = { a,a,a,	a,-a,a,		a,-a,-a,	a,a,-a,
		       -a,a,a,	-a,-a,a,	-a,-a,-a,	-a,a,-a };
  //GLfloat N_Cube[] = { a,a,a,	a,-a,a,		a,-a,-a,	a,a,-a,
  //		       -a,a,a,	-a,-a,a,	-a,-a,-a,	-a,a,-a };
  GLubyte I_Cube[] = { 0,1,2,3,	3,2,6,7,	2,1,5,6,	6,5,4,7,
		       7,4,0,3,	0,4,5,1 };

  float b = sqrt2/2;
  GLfloat V_Octus[] = { 0,0,1, b,b,0, -b,b,0, -b,-b,0, b,-b,0, 0,0,-1 };
  GLubyte I_Octus[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,1, 5,2,1, 5,3,2, 5,4,3, 5,1,4 };
}

namespace SphereTrings {
  GLfloat  CubeA	= a;
  GLfloat  OctusA	= sqrt2;

  GLfloat *Vertexen[5]	= { V_Tetrus, V_Cube, V_Octus };
  GLfloat *Normaleen[5]	= { V_Tetrus, V_Cube, V_Octus };
  GLubyte *Indexen[5]	= { I_Tetrus, I_Cube, I_Octus };
  GLenum   GLmode[5]	= { GL_TRIANGLES, GL_QUADS, GL_TRIANGLES };
  GLsizei  IndexSize[5]	= { 12, 24, 24 };
}

/**************************************************************************/
// These two set up and execute DrawElements ... you should enable texturing
//	or coloring yourself

using namespace SphereTrings;

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

/*
void SphereTrings::RenderEidos(ZTrans& t, EidosControl& ec)
{
  // first assume z as ka boo
  Real_t dtheta = TMath::Pi()/(ec.GetNTheta() + 1);
  Real_t dphi = 2*TMath::Pi()/(ec.GetNPhi() + 1);
  auto_ptr<ZVector> x = t.GetBase(1);
  auto_ptr<ZVector> y = t.GetBase(2);
  auto_ptr<ZVector> z = t.GetBase(3);
  ec.StartTube();
  ec.VertAdd(a); ec.EndLayer();
  for(UInt_t i=0; i<=ec.GetNTheta(); i++) {
    Real_t theta = dtheta*i;
    Real_t s_theta = TMath::Cos(theta);
    ZVector a = TMath::Cos(theta) * (*z);
    for(UInt_t j=0; j<=ec.GetNPhi(); j++) {
      phi = sphi*i;
      ec.VertAdd(a + s_theta*(TMath::Cos(phi)*(x) + TMath::Sin(phi)*(*y)));
    }
    ec.EndLayer();
  }
  ec.VertAdd(-a); ec.EndLayer();
  ec.EndTube();
}
*/

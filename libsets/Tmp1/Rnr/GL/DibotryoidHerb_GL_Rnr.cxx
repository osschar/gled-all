// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "DibotryoidHerb_GL_Rnr.h"
#include <GL/glew.h>

//==============================================================================

void DibotryoidHerb_GL_Rnr::_init()
{}

DibotryoidHerb_GL_Rnr::DibotryoidHerb_GL_Rnr(DibotryoidHerb* idol) :
  GrowingPlant_GL_Rnr(idol),
  mDibotryoidHerb(idol)
{
  _init();
}

DibotryoidHerb_GL_Rnr::~DibotryoidHerb_GL_Rnr()
{}

//==============================================================================
/*
void DibotryoidHerb_GL_Rnr::PreDraw(RnrDriver* rd) {}

void DibotryoidHerb_GL_Rnr::Draw(RnrDriver* rd) {}

void DibotryoidHerb_GL_Rnr::PostDraw(RnrDriver* rd) {}

void DibotryoidHerb_GL_Rnr::Render(RnrDriver* rd) {}
*/

//-------------------------------------------------------------------------------
void DibotryoidHerb_GL_Rnr::DrawStep(Turtle& turtle, GrowingPlant::Segment& p) 
{
  glLineWidth(p.mParam1 > 0 ? 4 : 1);
  
  glBegin(GL_LINES);
  float pos[3];
  turtle.mTrans.GetPos(pos);
  glVertex3fv(pos);
  turtle.mTrans.Move3LF(1, 0, 0);
  turtle.mTrans.GetPos(pos);
  glVertex3fv(pos);
  glEnd();
  
  glLineWidth(1);
}

void DibotryoidHerb_GL_Rnr::DrawSymbol(Turtle& turtle, GrowingPlant::Segment& p) 
{
  // Debug   
  glPushMatrix( );
  glMultMatrixd(turtle.mTrans.Array());
 // glRotatef(90, 0, 1, 0);
  
  if (p.mType == 'B')
  { 
    p.mParam1 = 0.1;
    glColor3f(0, 1, 0);
    gluSphere(mQuadric, 0.1 ,4, 4); 
    
   // printf("draw B \n");

  }
  else if (p.mType == 'K')
  { 
    p.mParam1 = 0.1;
    glColor3f(1, 1, 1);
    
    glBegin(GL_LINES);
    float a = 0.5;
    glVertex3f(0, 0, 0);
    glVertex3f(a, a, a);
    glEnd();
    
    glTranslatef(a, a, a);
    gluSphere(mQuadric, 0.3 ,4, 4); 
    // printf("Draw K \n");
  }
  
  else if (p.mType == 'L')
  {
    p.mParam1 = 1;
    glColor3f(1, 0, 0);
    float x = 0.21;
    gluCylinder(mQuadric, 0 , x, x, 8, 4);
    // printf("Draw L \n");
  } 
  
  else if (p.mType == 'a' || p.mType == 'b')
  {
    if (p.mType == 'a') 
      glColor3f(1, 0, 1); 
    else
      glColor3f(0, 1, 1);
    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    
    glEnd();
  }
  
  glPopMatrix();
}


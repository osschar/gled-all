// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GrowingPanicle_GL_Rnr.h"
#include <GL/glew.h>

//==============================================================================

void GrowingPanicle_GL_Rnr::_init()
{}

GrowingPanicle_GL_Rnr::GrowingPanicle_GL_Rnr(GrowingPanicle* idol) :
  GrowingPlant_GL_Rnr(idol),
  mGrowingPanicle(idol)
{
  _init();
// mLighting = false;
}

GrowingPanicle_GL_Rnr::~GrowingPanicle_GL_Rnr()
{}

//==============================================================================

void GrowingPanicle_GL_Rnr::DrawSymbol(Turtle& turtle, GrowingPlant::Segment& p) 
{
  glPushMatrix( );
  glMultMatrixd(turtle.mTrans.Array());
  
  if (p.mType == 'O')
  { 
//    glColor3f(TMath::Min(0.1f*p.mParam1, 1.f), 0.6 ,0.6);
 //   gluSphere(mQuadric, 0.05*TMath::Min(p.mParam2, 5) ,4, 4); 
    glColor3f(0.2*p.mParam1, 1 , 1);
    
    gluSphere(mQuadric, 0.1*TMath::Min(p.mParam2, 5) ,4, 4); 
    
    
    
  }
  else if(p.mType == 'G')
  {
    glColor3f(0, 1 ,0);
    gluSphere(mQuadric, 0.1 ,4, 4); 
  }
  
  else if(p.mType == 'A')
  {    
    glColor4fv(mGrowingPanicle->mLineColor.array());
    gluSphere(mQuadric, 0.1 ,4, 4); 
  }

  
  else if(p.mType == 'S')
  {
   // glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
    glRotatef(30, 0, 1, 0);
    glColor4fv(mGrowingPanicle->mSColor.array());
    DrawSignal(mGrowingPanicle->mSSize);
    //glPopAttrib();
  }
  
  else if(p.mType == 'T')
  {
    
    glRotatef(30, 0, 0, 1);
    glColor4fv(mGrowingPanicle->mTColor.array());
    DrawSignal(mGrowingPanicle->mTSize);
  }
  else if(p.mType == 'U')
  {
    glRotatef(50, 1, 1, 0);
    glColor4fv(mGrowingPanicle->mUColor.array());
    DrawSignal(mGrowingPanicle->mUSize);
  }
  
  
  glPopMatrix();
}

void GrowingPanicle_GL_Rnr::DrawSignal(float a) 
{
  
  glLineWidth(2);
  glBegin(GL_LINES);

  glVertex3f( -a,  0, 0);
  glVertex3f(  a,  0, 0);
  
  glVertex3f( 0,  0,  -a);
  glVertex3f( 0,  0,  +a);
  
  glVertex3f( 0, -a, 0);
  glVertex3f( 0, +a, 0);
  glEnd();
  glLineWidth(1);
}

//-------------------------------------------------------------------------------
void GrowingPanicle_GL_Rnr::DrawStep(Turtle& turtle, GrowingPlant::Segment& p) 
{
  if(p.mType == 'G')
    glColor3f(0, 1 , 1);
  else
    glColor3f(1, 1 , 0);
  
  bool linemode = true;
  if (linemode) glBegin(GL_LINES);
  float pos[3];
  turtle.mTrans.GetPos(pos);
  glVertex3fv(pos);
  
  turtle.mTrans.Move3LF(p.mParam2 ? 1:2, 0, 0);
  //printf("move %d \n");
  turtle.mTrans.GetPos(pos);
  glVertex3fv(pos);
  if (linemode) glEnd();
}
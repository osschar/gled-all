// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TVector3.h"
#include "DibotryoidHerb_GL_Rnr.h"
#include <GL/glew.h>

static int kLineWidth = 1;
static int kLineWidthB = 3;
//==============================================================================

void DibotryoidHerb_GL_Rnr::_init()
{
  mIAsForward = true;
}

DibotryoidHerb_GL_Rnr::DibotryoidHerb_GL_Rnr(DibotryoidHerb* idol) :
  GrowingPlant_GL_Rnr(idol),
  mDibotryoidHerb(idol)
{
  _init();
   mLighting = false;
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
  glColor4fv(mDibotryoidHerb->GetLineColor().array());
  
  ZPoint pos0 = turtle.mTrans.GetPos();
  ZPoint dir  = turtle.mTrans.GetBaseVec(1);
  ZPoint pos1 = pos0 + dir;
  
  glLineWidth(kLineWidth);
  glBegin(GL_LINES);
  glVertex3dv(&pos0[0]);   
  glVertex3dv(&pos1[0]);
  glEnd();
  
  glLineWidth(kLineWidthB);
  glBegin(GL_LINES);
  glVertex3dv(&pos0[0]);
  
  // param2 == -1 main, else lateral
  float delay = p.mParam2 == -1 ? mDibotryoidHerb->GetSignalDelayMainAxis() :  mDibotryoidHerb->GetSignalDelayLateralAxis();
  double f = p.mParam1*1.0f/delay;
  ZPoint pos2 = pos0;
  pos2 += f*dir;
  glVertex3dv(&pos2[0]);
  glEnd(); 
  
  turtle.mTrans.Move3LF(1, 0, 0);
}

void DibotryoidHerb_GL_Rnr::DrawSymbol(Turtle& turtle, GrowingPlant::Segment& p) 
{
  // Debug   
  glPushMatrix( );
  glMultMatrixd(turtle.mTrans.Array());
 // glRotatef(90, 0, 1, 0);
  
  if (p.mType == 'B')
  { 
    glColor3f(1, 1, 1);
    
    //gluSphere(mQuadric, 0.2 ,4, 4); 
    
    glLineWidth(kLineWidth);   
    glBegin(GL_LINES);
    /*
    float a = 0.5;
    glVertex3f(0, -a, -a);
    glVertex3f(0, a, a);
    
    glVertex3f(-a, -a, 0);
    glVertex3f(a, a, 0);
    */
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    
    glEnd();
   // printf("draw B \n");

  }
  else if (p.mType == 'K')
  {     
    glColor3f(1, 1, 1);
   // glColor4fv(mDibotryoidHerb->GetAColor().array());
    glLineWidth(kLineWidth);
    glBegin(GL_LINES);
    float a = 0.5;
    glVertex3f(0, 0, 0);
    glVertex3f(a, -a, 0);
    glEnd();
    
    glTranslatef(a, -a, 0);
    gluSphere(mQuadric, a*0.3 ,4, 4); 
    // printf("Draw K \n");
  }
  
  else if (p.mType == 'L')
  {
  //  glColor3f(1, 0, 0);
    glRotatef(90, 1, 1, 0);

    glColor4fv(mDibotryoidHerb->GetLColor().array());
    float x = 0.21;
    gluCylinder(mQuadric, 0 , x*0.2, x, 8, 4);
    // printf("Draw L \n");
  } 
  
  else if (p.mType == 'a' || p.mType == 'b')
  {
    float s = 0;
    if (p.mType == 'a') 
    {      
      glColor4fv(mDibotryoidHerb->GetAColor().array());
      s = p.mParam1*1.f/mDibotryoidHerb->mPlastocronMainAxis;
    }
    else
    {
      glColor4fv(mDibotryoidHerb->GetBColor().array());
      s = p.mParam1*1.f/mDibotryoidHerb->mPlastocronLateralAxis;
    }
    
    
    glLineWidth(kLineWidth);    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glEnd();
    
    glLineWidth(kLineWidthB);    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(s, 0, 0);
    glEnd();
    glLineWidth(1);
  }
  
  glPopMatrix();
}


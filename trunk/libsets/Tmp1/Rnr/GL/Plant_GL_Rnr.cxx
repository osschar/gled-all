// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Plant_GL_Rnr.h"
#include <GL/glew.h>
#include "TMath.h"


//==============================================================================

void Plant_GL_Rnr::_init()
{}

Plant_GL_Rnr::Plant_GL_Rnr(Plant* idol) :
  Weed_GL_Rnr(idol),
  mPlant(idol),
  mQuadric(0)
{
  mQuadric = gluNewQuadric();
}

Plant_GL_Rnr::~Plant_GL_Rnr()
{
  gluDeleteQuadric(mQuadric);
}

//-------------------------------------------------------------------------------
void Plant_GL_Rnr::Render(RnrDriver* rd) 
{
  SetStepSize();
  glPushAttrib(GL_CURRENT_BIT | GL_POLYGON_BIT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  ProcessExpression();
  glPopAttrib();
}

//-------------------------------------------------------------------------------
void Plant_GL_Rnr::SetStepSize()
{
  int nF = 0;
  for (int i = 0; i < mWeed->GetExpressionLength(); ++i)
  {
    if (mWeed->GetExpression()[i] == 'F') ++nF;
  }
  mStepSize = 1.f/TMath::Power(nF, 1/3.);  
}

//-------------------------------------------------------------------------------
void Plant_GL_Rnr::DecreaseWidth(Turtle& turtle) const
{
  turtle.lineWidth -= 0.1;
}

//-------------------------------------------------------------------------------
void Plant_GL_Rnr::DrawStep(Turtle& turtle) const
{  
  // cylinder
  glColor4fv(mPlant->GetLineColor().array());
  glPushMatrix( );
  glMultMatrixd(turtle.trans.Array());
  glRotatef(90, 0, 1, 0);
  double w = turtle.lineWidth * mPlant->mStemWidth * mStepSize;
  gluCylinder(mQuadric, w , w, mStepSize, 8, 4);
  glPopMatrix();
  
  // draw lines for debug
  {
    glColor3f(1, 0, 0);
    glPushMatrix( );
    glMultMatrixd(turtle.trans.Array());
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(mStepSize, 0, 0);
    glEnd();
    glPopMatrix();
  }
  
  turtle.trans.Move3LF(mStepSize, 0, 0);
}

//-------------------------------------------------------------------------------
void Plant_GL_Rnr::DrawLeaf(Turtle& turtle) const
{
  glColor3f(1, 0, 1);
  float ang = 2*mPlant->mAngle;
  turtle.trans.RotateLF(1, 3, ang); 
  
  glPushMatrix( );
  glMultMatrixd(turtle.trans.Array());
  turtle.trans.RotateLF(1, 3, -mPlant->mAngle);   
  double w = turtle.lineWidth * 0.2*mPlant->mStemWidth * mStepSize;
  float s = mPlant->mLeafSize*mStepSize;
  
  
  glColor3f(0, 1, 0);
  glPushMatrix();
  glRotatef(90, 0, 1, 0);
  gluCylinder(mQuadric, w , w, s, 8, 4); 
  glPopMatrix();
  
  glColor4fv(mPlant->GetLeafColor().array());
  glTranslatef(s, 0, 0);
  glBegin(GL_POLYGON);
  glVertex3f(0    , 0, 0);  
  glVertex3f(s/2  , s/4, 0);  
  glVertex3f(1.5*s, s/4, 0);
  glVertex3f(2*s, 0, 0);  
  glVertex3f(1.5*s, -s/4, 0); 
  glVertex3f(0.5*s, -s/4, 0); 
  
  glEnd();
  glPopMatrix();
  turtle.trans.RotateLF(1, 3, -ang); 
}

//-------------------------------------------------------------------------------
void Plant_GL_Rnr::DrawFlower(Turtle& turtle) const
{
  glColor4fv(mPlant->GetFlowerColor().array());
  glPushMatrix( );
  glMultMatrixd(turtle.trans.Array());
  glRotatef(90, 0, 1, 0);

  float s = mPlant->mFlowerSize*mStepSize;
  gluDisk(mQuadric, 0, s, 3,3);   
  glPopMatrix();
}

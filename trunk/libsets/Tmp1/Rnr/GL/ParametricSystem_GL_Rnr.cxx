// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ParametricSystem_GL_Rnr.h"
#include "TRandom.h"
#include <GL/glew.h>
#include <stack>
#include <Glasses/ParametricSystem.h>

//==============================================================================

void ParametricSystem_GL_Rnr::_init()
{}

ParametricSystem_GL_Rnr::ParametricSystem_GL_Rnr(ParametricSystem* idol) :
ZNode_GL_Rnr(idol),
mPS(idol),
mQuadric(0),
mScale(1.f),
mRandom(new TRandom),
mSeed(8889)
{
  _init();
  mQuadric = gluNewQuadric();
  /*
  mDLID = glGenLists(1);
  glNewList(mDLID, GL_COMPILE);
  gluCylinder(mQuadric, 1, 1, 1, 8, 4);
  glEndList();
   */
}

ParametricSystem_GL_Rnr::~ParametricSystem_GL_Rnr()
{
  gluDeleteQuadric(mQuadric);
}

//-------------------------------------------------------------------------------
void ParametricSystem_GL_Rnr::Render(RnrDriver* rd) 
{
  glEnable(GL_NORMALIZE);
  glPushMatrix();
  glScalef(mScale, mScale, mScale);
  printf("scale %f \n", mScale);
  glColor4fv(mPS->GetLineColor().array());
  ProcessExpression(true);

  glPopMatrix();
}

void ParametricSystem_GL_Rnr::Triangulate(RnrDriver* rd)
{ 
  
  BBoxInit();
  mPS->Produce();
  
  
  ProcessExpression(false);
  
  printf("extends %f \n",GetMaxExtend() );
  mScale = 1/GetMaxExtend();  
}

//-------------------------------------------------------------------------------
void ParametricSystem_GL_Rnr::ProcessExpression(bool draw) 
{
  mRandom->SetSeed(mSeed);
  
  std::stack<Turtle> stack;  
  Turtle init;
  Turtle& turtle = init;  
  stack.push(turtle);
  turtle.mWidth= 0.005;
  
  turtle.mLength= 1;
  std::vector<TwoParam> pv = mPS->GetExpression();

  //printf("==============================================================\n");
  for (std::vector<TwoParam>::iterator i = pv.begin(); i != pv.end(); ++i)
  {
 //   printf("param %c %f %f \n", (*i).mType, (*i).mParam1, (*i).mParam2);
    switch ((*i).mType) {        
        // stack
      case '[':
        stack.push(turtle);
        break;
      case ']':
        turtle = stack.top();
        stack.pop();
        break;
        
        // rotate around X axis
      case '+':
        turtle.mTrans.RotateLF(1, 2, (*i).mParam1*TMath::DegToRad());  
        break;
      case '-':
        turtle.mTrans.RotateLF(1, 2, -(*i).mParam1*TMath::DegToRad());   
        break;
      case '|':
        turtle.mTrans.RotateLF(1, 2, TMath::Pi());   
        break;
        
        // rotate around Y axis
      case '&':
        turtle.mTrans.RotateLF(1, 3, (*i).mParam1*TMath::DegToRad());  
        break;
      case '^':
        turtle.mTrans.RotateLF(1, 3, -(*i).mParam1*TMath::DegToRad());   
        break;
        
        // rotate around Z axis
      case '\\':
        turtle.mTrans.RotateLF(3, 2, (*i).mParam1*TMath::DegToRad());  
        break;
      case '/':
        turtle.mTrans.RotateLF(3, 2, -(*i).mParam1*TMath::DegToRad());   
        break;
        
      case '$':
        double frw[3];
        turtle.mTrans.GetBaseVec(1, frw);
        double grav[3];
        grav[0]= -1; grav[1] = 0; grav[2] =0;
        double left[3];
        TMath::Cross(frw, grav, left);
        turtle.mTrans.SetBaseVec(2, left);
        turtle.mTrans.Norm3Column(2);
        //turtle.mTrans.OrtoNorm3Column(1,2);
        turtle.mTrans.SetBaseVecViaCross(3);
       // turtle.mTrans.Dump();
        break;
        
        // tanslate
      case 'f':
        turtle.mTrans.Move3LF((*i).mParam1, 0, 0);
        break;  
      case 'F':
        DrawStep(turtle, *i, draw);
        break;
      case '!':
        turtle.mWidth = (*i).mParam1*TMath::Sqrt(3);
        printf("parametric system %f %f\n", turtle.mWidth, (*i).mParam1);
        break;
      default:
        break;
    } 
  }
}

//-------------------------------------------------------------------------------
void ParametricSystem_GL_Rnr::DrawStep(Turtle& turtle, TwoParam& p, bool draw) 
{
  double x = (1 - mPS->GetRndWeight()) +  mPS->GetRndWeight() * mRandom->Rndm();
  if (draw)
  {
    // cylinder
    const static float dec = TMath::Sqrt(2);
    
    glPushMatrix( );
    glMultMatrixd(turtle.mTrans.Array());
    glRotatef(90, 0, 1, 0);
    gluCylinder(mQuadric, p.mParam2*dec , p.mParam2, p.mParam1*x, 8, 4);
    glPopMatrix();  
  }
  
  turtle.mTrans.Move3LF(p.mParam1*x, 0, 0);
  
  if (!draw)
  {
    float pos[3];
    turtle.mTrans.GetPos(pos);
    BBoxCheckPoint(pos[0], pos[1], pos[2]);
  }
}

//-------------------------------------------------------------------------------
float ParametricSystem_GL_Rnr::GetMaxExtend()
{
  printf("%f %f %f \n", fBBox[1]-fBBox[0], fBBox[3]-fBBox[2], fBBox[5]-fBBox[4]);
  
  float s = fBBox[1]-fBBox[0];
  
  if (fBBox[3]-fBBox[2] > s)
  {
    s = fBBox[3]-fBBox[2];
  }
  
  if (fBBox[5]-fBBox[4] > s)
  {
    s = fBBox[5]-fBBox[4];
  }
  return s; 
}

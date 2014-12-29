// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TernaryTree_GL_Rnr.h"
#include <GL/glew.h>
#include <stack>
//==============================================================================

void TernaryTree_GL_Rnr::_init()
{}

TernaryTree_GL_Rnr::TernaryTree_GL_Rnr(TernaryTree* idol) :
  MonopodialTree_GL_Rnr(idol),
  mTT(idol)
{
  _init();
}

TernaryTree_GL_Rnr::~TernaryTree_GL_Rnr()
{}

//-------------------------------------------------------------------------------
void TernaryTree_GL_Rnr::ProcessExpression(bool draw) 
{
  std::stack<Turtle> stack;  
  Turtle init;
  Turtle& turtle = init;  
  stack.push(turtle);
  turtle.mWidth=  -1;
  turtle.mLength = -1;
  turtle.mChangedWidth = false;
  
  std::vector<TwoParam> pv = mTT->GetExpression();
  
  for (std::vector<TwoParam>::iterator i = pv.begin(); i != pv.end(); ++i)
  {
    switch ((*i).mType) {        
        // stack
      case '[':
        stack.push(turtle);
        break;
      case ']':
        turtle = stack.top();
        stack.pop();
        break;
        
        // rotate around Y axis
      case '&':
        turtle.mTrans.RotateLF(1, 3, (*i).mParam1*TMath::DegToRad());  
        break;
        
        // rotate around Z axis
      case '/':
        turtle.mTrans.RotateLF(3, 2, -(*i).mParam1*TMath::DegToRad());   
        break;
             
        // tanslate
      case 'F':
        DrawStep(turtle, *i, draw);
        break;
        
      case '!':
       // turtle.mWidth = (*i).mParam1;
        turtle.mChangedWidth = true;
        break;
      default:
        break;
    } 
  }
}

//==============================================================================

void TernaryTree_GL_Rnr::DrawStep(Turtle& turtle, TwoParam& p, bool draw) 
{
  if (1)
  {
    // base vector is 'torsion' system
    ZPoint v1 = turtle.mTrans.GetBaseVec(1).Cross(mTT->mTorsionNormalized);
    float ang = mTT->GetSusceptibility() * TMath::ASin(v1.Mag());
    
    float dot = turtle.mTrans.GetBaseVec(1).Dot(v1);
    ZPoint vP = v1;
    vP *= dot;
    ZPoint vT = turtle.mTrans.GetBaseVec(1) - vP;
    float vTMag = vT.Mag();
    vT *= 1/vTMag;
    
    ZPoint vRot = vT*TMath::Cos(ang) + mTT->mTorsionNormalized*TMath::Sin(ang);
    vRot *= vTMag;
    vRot += vP;
    
    turtle.mTrans.SetBaseVec(1, vRot);
  } 
  
  if (draw)
  {
    // cylinder
    glPushMatrix( );
    glMultMatrixd(turtle.mTrans.Array());
    glRotatef(90, 0, 1, 0);
    float w =  p.mParam2;//turtle.mWidth;
    float wf = w;
    float l = p.mParam1;
    
    gluCylinder(mQuadric, wf, w, l, 8, 4);    
    glPopMatrix();  
  }
  
  turtle.mTrans.Move3LF(p.mParam1, 0, 0);
  turtle.mChangedWidth = false;
}

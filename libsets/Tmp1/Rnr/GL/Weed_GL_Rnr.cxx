// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Weed_GL_Rnr.h"
#include <GL/glew.h>
#include "TMath.h"
#include "TVector3.h"
//==============================================================================

void Weed_GL_Rnr::_init()
{
}

Weed_GL_Rnr::Weed_GL_Rnr(Weed* idol) :
ZNode_GL_Rnr(idol),
mWeed(idol)
{
  _init();
}

Weed_GL_Rnr::~Weed_GL_Rnr()
{}

//==============================================================================

void Weed_GL_Rnr::Render(RnrDriver* rd) 
{
  glColor4fv(mWeed->mLineColor.array());
  SetStepSize();
  glBegin(GL_LINES);
  ProcessExpression();
  glEnd();
}

void Weed_GL_Rnr::Triangulate(RnrDriver* rd)
{
  mWeed->Produce();
}

//-------------------------------------------------------------------------------
void Weed_GL_Rnr::ProcessExpression() const
{
  std::stack<Turtle> stack;  Turtle init;
  Turtle& turtle = init;  
  stack.push(turtle);
  const char* ea =  mWeed->GetExpression();
  for (int i = 0; i <  mWeed->GetExpressionLength(); ++i)
  {
    switch (ea[i]) {        
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
        turtle.trans.RotateLF(1, 2, mWeed->mAngle);  
        break;
      case '-':
        turtle.trans.RotateLF(1, 2, -mWeed->mAngle);   
        break;
      case '|':
        turtle.trans.RotateLF(1, 2, TMath::Pi());   
        break;
        
        // rotate around Y axis
      case '&':
        turtle.trans.RotateLF(1, 3, mWeed->mAngle);  
        break;
      case '^':
        turtle.trans.RotateLF(1, 3, -mWeed->mAngle);   
        break;
        
        // rotate around Z axis
      case '\\':
        turtle.trans.RotateLF(3, 2, mWeed->mAngle);  
        break;
      case '/':
        turtle.trans.RotateLF(3, 2, -mWeed->mAngle);   
        break;
                
        // tanslate
      case 'f':
        turtle.trans.Move3LF(mStepSize, 0, 0);
        break;  
      case 'F':
        DrawStep(turtle);
        break;
      case 'L':
        DrawLeaf(turtle);
        break;
      case 'O':
        DrawFlower(turtle);
        break;        
      case '@':
        DecreaseWidth(turtle);
        break;
      default:
        break;
    } 
  }
}

//-------------------------------------------------------------------------------
void Weed_GL_Rnr::SetStepSize() 
{
  int nF = 0;
  const char* ea = mWeed->GetExpression();
  for (int i = 0; i < mWeed->GetExpressionLength(); ++i)
  {
    if (ea[i] == 'F') ++nF;
  }
  mStepSize = 1.f/TMath::Sqrt(nF);
}  

//-------------------------------------------------------------------------------
void Weed_GL_Rnr::DrawStep(Turtle& turtle) const
{
  double x, y, z;
  turtle.trans.GetPos(x, y, z);
  glVertex3d(x, y,z);
  turtle.trans.Move3LF(mStepSize, 0, 0);
  
  turtle.trans.GetPos(x, y, z);
  glVertex3d(x, y,z);
}

//-------------------------------------------------------------------------------
void Weed_GL_Rnr::DrawLeaf(Turtle& turtle) const
{
}

//-------------------------------------------------------------------------------
void Weed_GL_Rnr::DrawFlower(Turtle& turtle) const
{
}

//-------------------------------------------------------------------------------
void Weed_GL_Rnr::DecreaseWidth(Turtle& turtle) const
{
}



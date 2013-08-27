// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GrowingPlant_GL_Rnr.h"
#include "TRandom.h"
#include <GL/glew.h>
#include <stack>
#include <Glasses/GrowingPlant.h>
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/GLRnrDriver.h>
//==============================================================================

void GrowingPlant_GL_Rnr::_init()
{}

GrowingPlant_GL_Rnr::GrowingPlant_GL_Rnr(GrowingPlant* idol) :
ZNode_GL_Rnr(idol),
mModel(idol),
mQuadric(0),
mLighting(true),
mIAsForward(false)
{
  _init();
  mQuadric = gluNewQuadric();
}

GrowingPlant_GL_Rnr::~GrowingPlant_GL_Rnr()
{
  gluDeleteQuadric(mQuadric);
}

//-------------------------------------------------------------------------------
void GrowingPlant_GL_Rnr::Render(RnrDriver* rd) 
{
  //printf("render \n");
  glEnable(GL_NORMALIZE);

  glPushAttrib(GL_LINE_BIT);
  glColor4fv(mModel->GetLineColor().array());
  if (!mLighting) glDisable(GL_LIGHTING);
  ProcessExpression(rd);
  glPopAttrib();
}

void GrowingPlant_GL_Rnr::Triangulate(RnrDriver* rd)
{ 
  BBoxInit();
  mModel->Produce();  
}

//-------------------------------------------------------------------------------
bool linemode = true;

void GrowingPlant_GL_Rnr::ProcessExpression(RnrDriver* rd) 
{
  std::stack<Turtle> stack;  
  Turtle init;
  Turtle& turtle = init;  
  stack.push(turtle);

  int  cutBranch = 0;
  long idx = 0;
  for (GrowingPlant::Segments_i i = mModel->mSegments.begin(); i != mModel->mSegments.end(); ++i, ++idx)
  {
    if (cutBranch)
    {
      if ((*i).mType == '[')
      {
        cutBranch ++;
        // printf("++ cutBr %d \n", cutBranch);
        continue;
      }
      else if ((*i).mType == ']')
      {
        cutBranch--;
       // printf("-- cutBr %d \n", cutBranch);
        if (cutBranch)
          continue;
        
        
        glColor4fv(mModel->GetLineColor().array());
      }
      else if ((*i).mType == '%')
      {
        throw ("GrowingPlant_GL_Rnr::ProcessExpression() invalid stack opration.");
        cutBranch++;
      }
      else
      {
        // printf("pass  by %c \n", (*i).mType);
        continue;
      }
    }
    
    rd->GL()->PushName(this, reinterpret_cast<void*>(idx));
    
    switch ((*i).mType) { 
        
        // stack
      case '[':
        stack.push(turtle);
        //  printf("++ stack [%d]\n", stack.size());
        break;
      case ']':
        //   printf("__ stack [%d]\n", stack.size());
        glColor4fv(mModel->GetLineColor().array());
        turtle = stack.top();
        stack.pop();
        cutBranch = false;
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
        turtle.mTrans.RotateLF(3, 1, (*i).mParam1*TMath::DegToRad());  
        break;
      case '^':
        turtle.mTrans.RotateLF(3, 1, -(*i).mParam1*TMath::DegToRad());   
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
        turtle.mTrans.SetBaseVecViaCross(3);
        break;
  
        // translate
      case 'f':
        turtle.mTrans.Move3LF((*i).mParam1, 0, 0);
        break;  
      case 'F':
        DrawStep(turtle, *i);
        break;
        
      case 'I':
        if (mIAsForward)
           DrawStep(turtle, *i);
        break;
        
      case 'G':
        DrawSymbol(turtle, *i);

        turtle.mTrans.Move3LF(1, 0, 0);        // DrawStep(turtle, *i);
        break;
        
      case '\'':
        glColor4fv(mModel->GetFlowerColor().array());
        break; 
        
      case '{':
        glBegin(GL_TRIANGLE_FAN);
        linemode = false;
        break;
        
      case '}':
        glEnd();
        linemode = true;
        break;        
        
      case '.':
        float pos[3];
        turtle.mTrans.GetPos(pos);
        glVertex3fv(pos);
        break;
        
      case '%':
        cutBranch++;
        break;
      
      default:
        DrawSymbol(turtle, *i);
        break;
    }
    
    rd->GL()->PopName();
  }
}

//-------------------------------------------------------------------------------
void GrowingPlant_GL_Rnr::DrawStep(Turtle& turtle, GrowingPlant::Segment& p) 
{
  if (linemode) glBegin(GL_LINES);
  float pos[3];
  turtle.mTrans.GetPos(pos);
  glVertex3fv(pos);
  turtle.mTrans.Move3LF(1, 0, 0);
  turtle.mTrans.GetPos(pos);
  glVertex3fv(pos);
  if (linemode) glEnd();
}

void GrowingPlant_GL_Rnr::DrawSymbol(Turtle& turtle, GrowingPlant::Segment& p) 
{
  // Debug 
  /*
  glPushMatrix( );
  glMultMatrixd(turtle.mTrans.Array());
  glRotatef(90, 0, 1, 0);
  
  if (p.mType == 'K')
  { 
    p.mParam1 = 0.1;
    glColor3f(1, 1, 1);
    gluSphere(mQuadric, 0.1 ,4, 4); 
    // printf("draw K \n");
  }
  else if (p.mType == 'X')
  {
    p.mParam1 = 0.1;
    glColor3f(0, 0, 1);
  }
  else if (p.mType == 'L')
  {
    p.mParam1 = 1;
    glColor3f(1, 0, 0);
    float x = 0.051;
    gluCylinder(mQuadric, p.mParam1*x , p.mParam1*x, p.mParam1, 8, 4);
   // printf("Draw L \n");
  } 
   glPopMatrix();
   */
}



void GrowingPlant_GL_Rnr::HandlePick(RnrDriver* rd, lNSE_t& ns, lNSE_i nsi)
{
  // Only here as demonstration: trace pick event delivery.
  
  static const Exc_t _eh("DibotroidHerb_GL_Rnr::HandlePick ");
  
 // GrowingPlant::Segment* s = (GrowingPlant::Segment*) nsi->fUserData;
  // printf("%s Segmnet (%d, %d).\n", _eh.Data(), s->mParam1, s->mParam2);
  long idx = reinterpret_cast<long>(nsi->fUserData);
  printf("%s [%ld] > '%c' (%d, %d) \n",  _eh.Data(), idx, mModel->mSegments[idx].mType, mModel->mSegments[idx].mParam1, mModel->mSegments[idx].mParam2);
  
}

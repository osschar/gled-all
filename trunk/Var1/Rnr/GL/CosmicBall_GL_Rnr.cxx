// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CosmicBall_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

typedef ZNode_GL_Rnr PARENT_CLASS;

/**************************************************************************/

GLUquadricObj* CosmicBall_GL_Rnr::sQuadric = 0;

void CosmicBall_GL_Rnr::_init()
{
  if (sQuadric == 0)
  {
    sQuadric = gluNewQuadric();
    gluQuadricDrawStyle(sQuadric, GLU_FILL);
    gluQuadricNormals(sQuadric, GLU_SMOOTH);
  }
}

/**************************************************************************/

//void CosmicBall_GL_Rnr::PreDraw(RnrDriver* rd){}

void CosmicBall_GL_Rnr::Draw(RnrDriver* rd)
{
  CosmicBall& CB = *mCosmicBall;

  glPushAttrib(GL_CURRENT_BIT);
  rd->GL()->Color(CB.RefColor());
  gluSphere(sQuadric, CB.GetRadius(), CB.GetLOD(), CB.GetLOD());
  glPopAttrib();
}

void CosmicBall_GL_Rnr::PostDraw(RnrDriver* rd)
{
  PARENT_CLASS::PostDraw(rd);

  CosmicBall& CB = *mCosmicBall;

  GMutexHolder histo_lock(CB.mHistoryMoo);

  if (CB.mHistoryStored > 0)
  {
    glColor3f(0, 1, 0);
    glBegin(GL_LINE_STRIP);

    Int_t n_to_draw = CB.mHistoryStored;

    CosmicBall::Point* p;
    p = &CB.mHistory[CB.mHistoryFirst];
    Int_t lim_up = TMath::Min(CB.mHistoryFirst + n_to_draw, CB.mHistorySize);
    Int_t n_draw = lim_up - CB.mHistoryFirst;
    for (Int_t i = n_draw; i != 0; --i, ++p)
    {
      glVertex3fv(*p);
    }
    n_to_draw -= n_draw;

    if (n_to_draw > 0)
    {
      p = &CB.mHistory[0];
      for(Int_t i = n_to_draw; i != 0; --i, ++p)
      {
        glVertex3fv(*p);
      }
    }

    glEnd();
  }
}

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Amphitheatre_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

#include <TMath.h>

/**************************************************************************/

void Amphitheatre_GL_Rnr::_init()
{
  mQuadric = gluNewQuadric();
  gluQuadricDrawStyle(mQuadric, GLU_SILHOUETTE);
  gluQuadricNormals(mQuadric, GLU_FLAT);
}

/**************************************************************************/
/*
void Amphitheatre_GL_Rnr::PreDraw(RnrDriver* rd)
{}
*/

void Amphitheatre_GL_Rnr::Draw(RnrDriver* rd)
{
  Amphitheatre& A = *mAmphitheatre;

  if(A.bRnrStage) {
    rd->GL()->Color(A.mStageCol);
    glPushMatrix();
    glRotated((0.5 + A.mStageRot) * 360 / A.mStageSides - 90, 0, 0, 1);
    gluDisk(mQuadric, 0, A.mStageSize, A.mStageSides, 1);
    glPopMatrix();
  }

  if(A.bRnrChairs) {
    rd->GL()->Color(A.mChairCol);
    for(Amphitheatre::lChair_i i = A.mChairs.begin();
	i != A.mChairs.end(); ++i) 
      {
	glPushMatrix();
	TVector3& p( i->fPos );
	Double_t phi = TMath::ATan2(p.y(), p.x()) * 180 / TMath::Pi();
	glTranslated(p.x(), p.y(), p.z());
	glRotated(phi, 0, 0, 1);
	gluPartialDisk(mQuadric, 0, A.mChairSize, 7, 1, -45, 270);
	glPopMatrix();
      }
  }
}

/*
void Amphitheatre_GL_Rnr::PostDraw(RnrDriver* rd)
{}
*/

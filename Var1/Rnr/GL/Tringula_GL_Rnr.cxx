// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Tringula_GL_Rnr.h"
#include <Rnr/GL/TringTvor_GL_Rnr.h>
#include <Glasses/ZHashList.h>
#include <Glasses/TriMesh.h>
#include <Glasses/Dynamico.h>

#include <Rnr/GL/GLRnrDriver.h>

#include <Opcode/Opcode.h>

#include <GL/gl.h>

#define PARENT ZNode_GL_Rnr

/**************************************************************************/

void Tringula_GL_Rnr::_init()
{  mQuadric = gluNewQuadric();
  gluQuadricDrawStyle(mQuadric, GLU_LINE);
  gluQuadricNormals(mQuadric, GLU_NONE);
}

Tringula_GL_Rnr::~Tringula_GL_Rnr()
{
  gluDeleteQuadric(mQuadric);
}

/**************************************************************************/

namespace {
void sphere(Float_t* v, GLUquadricObj* q)
{
  glPushMatrix();
  glTranslatef(v[0], v[1], v[2]);
  gluSphere(q, 0.02, 5, 5);
  glPopMatrix();
}
}

void Tringula_GL_Rnr::Draw(RnrDriver* rd)
{
  Tringula&   T = *mTringula;
  TringTvor* TT =  T.mMesh->GetTTvor();
  if(TT == 0) return;
  PARENT::Draw(rd);

  if(T.bRnrRay) {
    { // Render Ray
      glPushMatrix();
      glTranslated(T.mRayPos.x(), T.mRayPos.y(),T.mRayPos.z());
      glColor3f(1,0,0);
      glPointSize(10);
      glBegin(GL_POINTS); glVertex3f(0,0,0); glEnd();
      glColor3f(0,1,0);
      glLineWidth(2);
      glBegin(GL_LINES);
      glVertex3f(0,0,0);
      Float_t d[3]; T.get_ray_dir(d);
      glVertex3fv(d);
      glEnd();
      glLineWidth(1);
      glPopMatrix();
    }
    // Render stabbed triangle
    if(T.mOPCCFaces != 0) {
      using namespace Opcode;
      CollisionFaces& CF = *T.mOPCCFaces;
      for(UInt_t f=0; f<CF.GetNbFaces(); ++f) {
        const CollisionFace& cf = CF.GetFaces()[f];
        Int_t   *t  = TT->Triangle(cf.mFaceID);
        Float_t *v0 = TT->Vertex(t[0]);
        Float_t *v1 = TT->Vertex(t[1]);
        Float_t *v2 = TT->Vertex(t[2]);

        glColor3f(1,0,0); sphere(v0, mQuadric);
        glColor3f(0,1,0); sphere(v1, mQuadric);
        glColor3f(0,0,1); sphere(v2, mQuadric);

        Point e1(v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]);
        Point e2(v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]);

        Point i(v0); i += cf.mU*e1 + cf.mV*e2;
        glColor3f(1,1,0); sphere((Float_t*)&i, mQuadric);

        /*
        // For test, calculate back the u,v coords.
        // This works OK.
        Float_t e1sq = e1.SquareMagnitude();
        Float_t e2sq = e2.SquareMagnitude();
        Point p      = cf.mU*e1 + cf.mV*e2;
        Float_t d    = e1 | e2;
        Float_t e1p  = e1 | p;
        Float_t e2p  = e2 | p;
        Float_t u = (e1p * e2sq - e2p * d) / (e1sq * e2sq - d * d);
        Float_t v = (e2p - u * d) / e2sq;
        printf("Recalc u=%f, v=%f\n", u, v);
        */
      }
    }
  }

  if (T.bRnrDynos) {
    AList::Stepper<Dynamico> stepper(*T.mDynos);
    while (stepper.step()) {
      Dynamico& D = **stepper;
      if (!D.GetRnrSelf()) continue;

      glPushMatrix();

      glMultMatrixd(D.RefTrans().Array());
      // This name-stack stuff here is foocked !!!!
      if (T.bPickDynos) rd->GL()->PushName(rd->GetLensRnr(&D));
      TringTvor_GL_Rnr::Render(D.GetMesh()->GetTTvor());
      if (T.bPickDynos) rd->GL()->PopName();

      glPopMatrix();
    }
  }
}

void Tringula_GL_Rnr::Render(RnrDriver* rd)
{
  // Checked in Draw() !!!
  // if(mTringula->mTTvor == 0) return;

  Tringula  &T  = *mTringula;
  TringTvor &TV = *T.mMesh->GetTTvor();
  glColor4fv(T.mColor());
  TV.bSmoothShade = (TV.mNorms && (T.bPreferSmooth || TV.mTringNorms == 0));
  TringTvor_GL_Rnr::Render(&TV);
}

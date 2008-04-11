// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Tringula_GL_Rnr.h"
#include <Rnr/GL/TringTvor_GL_Rnr.h>
#include <Glasses/ZHashList.h>
#include <Glasses/TriMesh.h>
#include <Glasses/Statico.h>
#include <Glasses/Dynamico.h>

#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/SphereTrings.h>

#include <Opcode/Opcode.h>

#include <GL/gl.h>

#define PARENT ZNode_GL_Rnr

/**************************************************************************/

void Tringula_GL_Rnr::_init()
{
  mQuadric = gluNewQuadric();
  gluQuadricDrawStyle(mQuadric, GLU_LINE);
  gluQuadricNormals(mQuadric, GLU_NONE);

  mMeshTringStamp = 0;
}

Tringula_GL_Rnr::~Tringula_GL_Rnr()
{
  gluDeleteQuadric(mQuadric);
}

/**************************************************************************/

namespace {

void sphere(const Float_t* v, GLUquadricObj* q)
{
  glPushMatrix();
  glTranslatef(v[0], v[1], v[2]);
  gluSphere(q, 0.02, 5, 5);
  glPopMatrix();
}

void render_ceaabox(const Float_t* x, Float_t f=1)
{
  // Render center-extents axis-aligned bounding-box
  f *= 2;
  glPushMatrix();
  glTranslatef(x[0]-x[3], x[1]-x[4], x[2]-x[5]);
  glScalef(f*x[3], f*x[4], f*x[5]);
  SphereTrings::UnitFrameBox();
  glPopMatrix();
}

}


void Tringula_GL_Rnr::RenderExtendio(RnrDriver* rd, Extendio* ext)
{
  glPushMatrix();

  glMultMatrixf(ext->RefLastTrans().Array());
  // !!! This is a horrible hack ... hash_lookup for nothing.
  // !!! Should use secondary selection (internal id-resolution).
  if (mTringula->bPickDynos) rd->GL()->PushName(rd->GetLensRnr(ext));

  TringTvor_GL_Rnr::Render(ext->GetMesh()->GetTTvor(), false);

  if (mTringula->bRnrBBoxes)
  {
    GL_Capability_Switch ligt_off(GL_LIGHTING, false);
    glColor3f(1, 0, 0);
    render_ceaabox(ext->GetMesh()->GetTTvor()->mCtrExtBox, 1.01);
  }

  if (mTringula->bPickDynos) rd->GL()->PopName();

  glPopMatrix();

  if (mTringula->bRnrBBoxes)
  {
    GL_Capability_Switch ligt_off(GL_LIGHTING, false);
    glColor3f(0, 0, 1);
    render_ceaabox((Float_t*)&ext->RefLastAABB(), 1.01);
  }
}

void Tringula_GL_Rnr::RenderExtendios(RnrDriver* rd, AList* list)
{
  AList::Stepper<Extendio> stepper(list);
  while (stepper.step())
  {
    if (! (*stepper)->GetRnrSelf()) continue;
    RenderExtendio(rd, *stepper);
  }
}

/******************************************************************************/

void Tringula_GL_Rnr::Draw(RnrDriver* rd)
{
  Tringula&   T = *mTringula;
  TringTvor* TT =  T.mMesh->GetTTvor();
  if(TT == 0) return;
  if(mMeshTringStamp < T.mMesh->GetStampReqTring())
  {
    bRebuildDL = true;
    mMeshTringStamp = T.mMesh->GetStampReqTring();
  }
  PARENT::Draw(rd);

  if(T.bRnrRay)
  {
    // Render Ray
    glPushMatrix();
    glTranslated(T.mRayPos.x(), T.mRayPos.y(), T.mRayPos.z());
    {
      GL_Float_Holder point_size(GL_POINT_SIZE, 10, glPointSize);
      glColor3f(1,0,0);
      glBegin(GL_POINTS); glVertex3f(0,0,0); glEnd();
    }
    glColor3f(0,1,0);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    Float_t d[3]; T.get_ray_dir(d);
    glVertex3fv(d);
    glEnd();
    glLineWidth(1);
    glPopMatrix();

    // Render stabbed triangle(s)
    if(T.mRayColFaces != 0)
    {
      using namespace Opcode;
      CollisionFaces& CF = *T.mRayColFaces;
      for(UInt_t f=0; f<CF.GetNbFaces(); ++f)
      {
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

  if (T.bRnrDynos)
  {
    RenderExtendios(rd, *T.mStatos);
    RenderExtendios(rd, *T.mDynos);
    RenderExtendios(rd, *T.mFlyers);

    if (T.bRnrItsLines)
    {
      GL_Capability_Switch ligt_off(GL_LIGHTING, false);
      GL_Float_Holder      line_w(GL_LINE_WIDTH, 2, glLineWidth);
      glColor3f(1, 0.1, 0.3);
      glBegin(GL_LINES);
      for (vector<Opcode::Segment>::iterator i = T.mItsLines.begin(); i != T.mItsLines.end(); ++i)
      {
        glVertex3fv(i->mP0);
        glVertex3fv(i->mP1);
      }
      glEnd();
    }
  }
}

void Tringula_GL_Rnr::Render(RnrDriver* rd)
{
  Tringula  &T  = *mTringula;
  TringTvor &TT = *T.mMesh->GetTTvor();
  glColor4fv(T.mColor());
  GL_Capability_Switch light(GL_LIGHTING, T.bLightMesh);
  if (T.bSmoothShade)
    assert(TT.HasNorms());
  else
    assert(TT.HasTringNorms());
  TringTvor_GL_Rnr::Render(&TT, T.bSmoothShade);
}

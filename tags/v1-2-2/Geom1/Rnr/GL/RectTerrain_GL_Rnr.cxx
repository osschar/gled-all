// $#Header$
#include "RectTerrain_GL_Rnr.h"

#include <GL/glu.h>

RectTerrain_GL_Rnr::RectTerrain_GL_Rnr(RectTerrain* rt) :
  ZNode_GL_Rnr(rt), mTerrain(rt)
{}

/**************************************************************************/

void RectTerrain_GL_Rnr::check(double xf, double yf,
			       GLdouble* pm, GLdouble* mm, GLint* v)
{
  GLdouble x,y,z;
  int ret = gluProject(xf*mTerrain->mNx*mTerrain->mDx,
		       yf*mTerrain->mNy*mTerrain->mDy,
		       0,
		       mm, pm, v, &x, &y, &z);
  printf("RectTerr: [%4.1f,%4.1f] %1d x=%7.2f y=%7.2f z=%7.2f\n",
	 xf, -yf, ret, x,y,z);
}

void RectTerrain_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mTerrain->mNx) {

    if(mTerrain->bStudySize) {
      GLdouble proj_m[16], modv_m[16];
      GLint    viewp[4];
      glGetDoublev(GL_MODELVIEW_MATRIX, modv_m);
      glGetDoublev(GL_PROJECTION_MATRIX, proj_m);
      glGetIntegerv(GL_VIEWPORT, viewp);
      printf("************************************************************************\n");
      printf("Viewport %4d %4d %4d %4d\n", viewp[0], viewp[1], viewp[2], viewp[3]);
      check(0,0, proj_m, modv_m, viewp);
      check(1,1, proj_m, modv_m, viewp);
      check(1,0, proj_m, modv_m, viewp);
      check(0,1, proj_m, modv_m, viewp);
      check(0.5,0.5, proj_m, modv_m, viewp);
    }

    glPushMatrix();
    glScalef(1,1,mTerrain->mZScale);
    glEnable(GL_NORMALIZE);

    glColor4fv(mTerrain->mMinCol());
    for(UCIndex_t i=0; i<mTerrain->mNx-1; i++) {
      glBegin(GL_QUAD_STRIP);
      for(UCIndex_t j=0; j<mTerrain->mNy; j++) {
	MkVN(i, j); MkVN(i+1u, j);
      }
      glEnd();
    }

    glPopMatrix();
    glDisable(GL_NORMALIZE);

  }
}

/**************************************************************************/

void RectTerrain_GL_Rnr::MkVN(UCIndex_t i, UCIndex_t j)
{
  if(mTerrain->mColSep) {
    Real_t c = (mTerrain->mP(i,j) - mTerrain->mMinZ) * mTerrain->mColSep /
      (mTerrain->mMaxZ - mTerrain->mMinZ);
    Real_t c1 = c - (int)c, c2 = 1 - c1;
    ZColor col( c2*mTerrain->mMinCol + c1*mTerrain->mMaxCol );
    glColor4fv(col());
  }
  Real_t x, y, z;

  x = mTerrain->mOx + i*mTerrain->mDx;
  y = mTerrain->mOy + j*mTerrain->mDy;
  z = mTerrain->mP(i,j);

  UCIndex_t il=i,ih=i,jh=j,jl=j;
  if(i>0) il--; if(i<mTerrain->mNx-1) ih++;
  if(j>0) jl--; if(j<mTerrain->mNy-1) jh++;
  Float_t nx[] = { mTerrain->mDx*(ih-il), 0, mTerrain->mP(ih,j)-mTerrain->mP(il,j) };
  Float_t ny[] = { 0, mTerrain->mDy*(jh-jl), mTerrain->mP(i,jh)-mTerrain->mP(i,jl) };

  glNormal3f(-ny[1]*nx[2], -nx[0]*ny[2], nx[0]*ny[1]);
  glVertex3f(x, y, z);
}

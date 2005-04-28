// $#Header$
#include "RectTerrain_GL_Rnr.h"

#include <GL/glu.h>

#define PARENT ZNode_GL_Rnr

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

/**************************************************************************/

void RectTerrain_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mTerrain->mNx && mTerrain->mNy) {

    switch(mTerrain->mOriginMode) {
    case RectTerrain::OM_Border:
      glTranslatef(mTerrain->mDx, mTerrain->mDy, 0);
      break;
    case RectTerrain::OM_Center:
      glTranslatef(-0.5*mTerrain->mNx*mTerrain->mDx,
		   -0.5*mTerrain->mNy*mTerrain->mDy, 0);
      break;
    default:
      break;
    }

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

    PARENT::Draw(rd);
  }
}

/**************************************************************************/

// #include <TRandom.h>

void RectTerrain_GL_Rnr::Render(RnrDriver* rd)
{
  switch(mTerrain->mRnrMode) {

  case RectTerrain::RM_Histo: {
    glColor4fv(mTerrain->mMinCol());
    Int_t minX = 1, maxX = mTerrain->mNx, minY = 1, maxY = mTerrain->mNy;
    if(mTerrain->bBorder) {
      --minX; ++maxX; --minY; ++maxY;
    }
    glBegin(GL_QUADS);
    for(Int_t i=minX; i<=maxX; ++i) {
      for(Int_t j=minY; j<=maxY; ++j) {
	MkBox(i, j);
      }
    }
    glEnd();
    break;
  }

  case RectTerrain::RM_SmoothSquare: {
    Int_t minX = 1, maxX = mTerrain->mNx, minY = 1, maxY = mTerrain->mNy;
    if(mTerrain->bBorder) {
      --minX; ++maxX; --minY; ++maxY;
    }
    glEnable(GL_NORMALIZE);
    glColor4fv(mTerrain->mMinCol());
    for(Int_t i=minX; i<maxX; ++i) {
      glBegin(GL_QUAD_STRIP);
      for(Int_t j=minY; j<=maxY; ++j) {
	MkVN(i, j); MkVN(i+1, j);
      }
      glEnd();
    }
    glDisable(GL_NORMALIZE);
    break;
  }

  case RectTerrain::RM_SmoothTring: {
    mTerrain->Tringoo();
    RectTringTvor& RTT = *mTerrain->pRTTvor;

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glVertexPointer(3, GL_FLOAT, 0, RTT.mVerts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, RTT.mNorms);
    glEnableClientState(GL_NORMAL_ARRAY);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, RTT.mCols);
    glEnableClientState(GL_COLOR_ARRAY);

    // TRandom colgen(2);

    if(mTerrain->bUseTringStrips) {
      // Requires GL-1.4
      // glMultiDrawElements(GL_TRIANGLE_STRIP, RTT.mStripLens,
      //                     GL_UNSIGNED_INT, RTT.mStripBegs, RTT.mNStrips);

      for(Int_t i=0; i<RTT.mNStrips; ++i) {
	// glColor3f(0.2+0.8*colgen.Rndm(), 0.2+0.8*colgen.Rndm(), 0.2+0.8*colgen.Rndm());
	glDrawElements(GL_TRIANGLE_STRIP, RTT.mStripLens[i],
		       GL_UNSIGNED_INT, RTT.mStripBegs[i]);
      }

    } else {

      glDrawElements(GL_TRIANGLES, RTT.mNTrings*3,
		     GL_UNSIGNED_INT, RTT.mTrings);

    }
    glPopClientAttrib();
    break;
  }

  case RectTerrain::RM_FlatTring: {
    mTerrain->Tringoo();
    RectTringTvor& RTT = *mTerrain->pRTTvor;

    GLint ex_shade_model;
    glGetIntegerv(GL_SHADE_MODEL, &ex_shade_model);
    glShadeModel(GL_FLAT);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glVertexPointer(3, GL_FLOAT, 0, RTT.mVerts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, RTT.mCols);
    glEnableClientState(GL_COLOR_ARRAY);

    // TRandom colgen(2);

    if(mTerrain->bUseTringStrips) {

      for(Int_t i=0; i<RTT.mNStrips; ++i) {
	glBegin(GL_TRIANGLE_STRIP);
	Int_t* idxp = RTT.mStripBegs[i];

	glArrayElement(*(idxp++));
	glArrayElement(*(idxp++));
	
	Int_t* tring_idxp = RTT.mStripTrings + (idxp - RTT.mStripEls);
	Int_t  n          = RTT.mStripLens[i] - 2;
	// glColor3f(0.2+0.8*colgen.Rndm(), 0.2+0.8*colgen.Rndm(), 0.2+0.8*colgen.Rndm());
	while(n-- > 0) {
	  glNormal3fv( RTT.TriangleNormal(*(tring_idxp++)) ); 
	  glArrayElement(*(idxp++));
	}
	glEnd();
      }

    } else {

      glBegin(GL_TRIANGLES);
      for(Int_t t=0; t<RTT.mNTrings; ++t) {
	Int_t*   T = RTT.Triangle(t);
	Float_t* N = RTT.TriangleNormal(t);
	glNormal3fv(N);
	glArrayElement(T[0]);
	glArrayElement(T[1]);
	glArrayElement(T[2]);
      }
      glEnd();

    }
    glPopClientAttrib();
    glShadeModel(ex_shade_model);
    break;
  }

  } // end switch Render Mode
}

/**************************************************************************/

void RectTerrain_GL_Rnr::MkBox(Int_t i, Int_t j)
{
  Float_t x, y, z;

  x = (i-1)*mTerrain->mDx;
  y = (j-1)*mTerrain->mDy;
  z = mTerrain->mP(i,j);

  MkCol(z);
  glNormal3f(0, 0, 1);
  glVertex3f(x, y, z);
  glVertex3f(x + mTerrain->mDx, y, z);
  glVertex3f(x + mTerrain->mDx, y + mTerrain->mDy, z);
  glVertex3f(x, y + mTerrain->mDy, z);
}

void RectTerrain_GL_Rnr::MkVN(Int_t i, Int_t j)
{
  Float_t x, y, z;

  x = (i-1)*mTerrain->mDx;
  y = (j-1)*mTerrain->mDy;
  z = mTerrain->mP(i,j);

  Int_t il=i,ih=i,jh=j,jl=j;
  if(i>0) il--; if(i<=mTerrain->mNx) ih++;
  if(j>0) jl--; if(j<=mTerrain->mNy) jh++;
  Float_t nx[] = { (ih-il)*mTerrain->mDx, 0, mTerrain->mP(ih,j) - mTerrain->mP(il,j) };
  Float_t ny[] = { 0, (jh-jl)*mTerrain->mDy, mTerrain->mP(i,jh) - mTerrain->mP(i,jl) };

  MkCol(z);
  glNormal3f(-ny[1]*nx[2], -nx[0]*ny[2], nx[0]*ny[1]);
  glVertex3f(x, y, z);
}

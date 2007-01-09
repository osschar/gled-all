// $#Header$
#include "RectTerrain_GL_Rnr.h"
#include "TringTvor_GL_Rnr.h"

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

void RectTerrain_GL_Rnr::setup_coltex_vars()
{
  RectTerrain& T = *mTerrain;
  bColP = (T.mColSep  != 0);
  bTexP = (T.mTexture.is_set());
  mMinX = 1; mMaxX = T.mNx; mMinY = 1; mMaxY = T.mNy;
  if(T.bBorder) {
    --mMinX; ++mMaxX; --mMinY; ++mMaxY;
  }
  mTexFx = 1.0/(mMaxX - mMinX); mTexFy = 1.0/(mMaxY - mMinY);
  mTexDx = T.mDx*mTexFx;        mTexDy = T.mDy*mTexFy;
}

/**************************************************************************/

void RectTerrain_GL_Rnr::Draw(RnrDriver* rd)
{
  RectTerrain& T = *mTerrain;
  if(T.mNx && T.mNy) {

    switch(mTerrain->mOriginMode) {
    case RectTerrain::OM_Border:
      glTranslatef(T.mDx, T.mDy, 0);
      break;
    case RectTerrain::OM_Center:
      glTranslatef(-0.5*T.mNx*T.mDx,
		   -0.5*T.mNy*T.mDy, 0);
      break;
    default:
      break;
    }

    if(T.bStudySize) {
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

void RectTerrain_GL_Rnr::Render(RnrDriver* rd)
{
  RectTerrain& T = *mTerrain;
  switch(T.mRnrMode) {

  case RectTerrain::RM_Histo: {
    setup_coltex_vars();
    glColor4fv(T.mMinCol());
    glBegin(GL_QUADS);
    for(Int_t i=mMinX; i<=mMaxX; ++i) {
      for(Int_t j=mMinY; j<=mMaxY; ++j) {
	MkBox(i, j);
      }
    }
    glEnd();
    break;
  }

  case RectTerrain::RM_SmoothSquare: {
    setup_coltex_vars();
    glEnable(GL_NORMALIZE);
    glColor4fv(mTerrain->mMinCol());
    for(Int_t i=mMinX; i<mMaxX; ++i) {
      glBegin(GL_QUAD_STRIP);
      for(Int_t j=mMinY; j<=mMaxY; ++j) {
	MkVN(i, j); MkVN(i+1, j);
      }
      glEnd();
    }
    glDisable(GL_NORMALIZE);
    break;
  }

  case RectTerrain::RM_SmoothTring:
  case RectTerrain::RM_FlatTring:   {
    if(T.mTTvorStamp < T.mStampReqTring)
      T.MakeTringTvor();
    glColor4fv(T.mMinCol());
    if (T.mRnrMode == RectTerrain::RM_SmoothTring)
      TringTvor_GL_Rnr::RenderSmooth(T.pTTvor);
    else
      TringTvor_GL_Rnr::RenderFlat(T.pTTvor);

    break;
  }

  } // end switch Render Mode
}

/**************************************************************************/

void RectTerrain_GL_Rnr::MkBox(Int_t i, Int_t j)
{
  RectTerrain& T = *mTerrain;

  Float_t x = (i-1)*T.mDx, y = (j-1)*T.mDy, z = T.mP(i,j);

  glNormal3f(0, 0, 1);
  if(bColP)
    glColor4fv(T.make_color(z)());

  if(bTexP) {
    Float_t tx = mTexFx*(i-mMinX), ty = mTexFy*(j-mMinY);
    glTexCoord2f(tx,        ty);        glVertex3f(x, y, z);
    glTexCoord2f(tx+mTexDx, ty);        glVertex3f(x + T.mDx, y, z);
    glTexCoord2f(tx+mTexDx, ty+mTexDy); glVertex3f(x + T.mDx, y + T.mDy, z);
    glTexCoord2f(tx,        ty+mTexDy); glVertex3f(x, y + T.mDy, z);
  } else {
    glVertex3f(x, y, z);
    glVertex3f(x + T.mDx, y, z);
    glVertex3f(x + T.mDx, y + T.mDy, z);
    glVertex3f(x, y + T.mDy, z);
  }
}

void RectTerrain_GL_Rnr::MkVN(Int_t i, Int_t j)
{
  RectTerrain& T = *mTerrain;

  Float_t x = (i-1)*T.mDx, y = (j-1)*T.mDy, z = T.mP(i,j);

  Int_t il=i, ih=i, jh=j, jl=j;
  if(i > 0) il--; if(i <= T.mNx) ih++;
  if(j > 0) jl--; if(j <= T.mNy) jh++;
  Float_t nx[] = { (ih-il)*T.mDx, 0, T.mP(ih,j) - T.mP(il,j) };
  Float_t ny[] = { 0, (jh-jl)*T.mDy, T.mP(i,jh) - T.mP(i,jl) };

  glNormal3f(-ny[1]*nx[2], -nx[0]*ny[2], nx[0]*ny[1]);
  if(bColP) glColor4fv(T.make_color(z)());
  if(bTexP) glTexCoord2f(mTexFx*(i-mMinX), mTexFy*(j-mMinY));
  glVertex3f(x, y, z);
}

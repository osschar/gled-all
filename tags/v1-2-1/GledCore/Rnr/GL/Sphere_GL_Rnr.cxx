// $#Header$
#include "Sphere_GL_Rnr.h"


void Sphere_GL_Rnr::_init() {
  mQuadric = gluNewQuadric();
  gluQuadricDrawStyle(mQuadric, GLU_FILL);
  gluQuadricNormals(mQuadric, GLU_SMOOTH);
}

Sphere_GL_Rnr::~Sphere_GL_Rnr()
{
  gluDeleteQuadric(mQuadric);
}

/**************************************************************************/

void Sphere_GL_Rnr::Draw(RnrDriver* rd)
{
  glPushAttrib(GL_CURRENT_BIT);
  glColor4fv(mSphere->mColor());
  gluSphere(mQuadric, mSphere->mRadius, mSphere->mLOD, mSphere->mLOD);
  glPopAttrib();
}

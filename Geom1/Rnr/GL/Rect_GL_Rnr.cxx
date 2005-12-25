// $#Header$
#include "Rect_GL_Rnr.h"
#include "GL/gl.h"

void Rect_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mRect->mWidth <= 0) return;

  glPushAttrib(GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
  glPushMatrix();

  // Strips
  if(mRect->mWidth)
    glLineWidth(mRect->mWidth);
  glColor4fv(mRect->mColor());
  glBegin(GL_LINES);
  const Float_t mX = 0.5*mRect->mULen, mY = 0.5*mRect->mVLen;
  if(mRect->mUStrips) {
    Float_t step=mRect->mULen/mRect->mUStrips, u=-mX;
    for(Short_t i=0; i<=mRect->mUStrips; ++i) {
      glVertex3f(u, -mY, 0); glVertex3f(u, mY, 0);
      u += step;
    }
  }
  if(mRect->mVStrips) {
    Float_t step=mRect->mVLen/mRect->mVStrips, v=-mY;
    for(Short_t i=0; i<=mRect->mVStrips; ++i) {
      glVertex3f(-mX, v, 0); glVertex3f(mX, v, 0);
      v += step;
    }
  }
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

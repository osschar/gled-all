// $#Header$
#include "Rect_GL_Rnr.h"
#include "FL/gl.h"

void Rect_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mRect->mWidth <= 0) return;

  glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT);
  glPushMatrix();
  glDisable(GL_LIGHTING);
  glScalef(mRect->mULen, mRect->mVLen, 1);

  // Strips
  glLineWidth(mRect->mWidth);
  glColor4fv(mRect->mColor());
  glNormal3f(0,0,1);
  if(mRect->mUStrips) {
    Float_t step=1.0/mRect->mUStrips, u=-0.5;
    glBegin(GL_LINES);
    for(unsigned short i=0; i<=mRect->mUStrips; ++i) {
      glVertex3f(u,-0.5,0); glVertex3f(u,0.5,0);
      u += step;
    }
    glEnd();
  }
  if(mRect->mVStrips) {
    Float_t step=1.0/mRect->mVStrips, v=-0.5;
    glBegin(GL_LINES);
    for(unsigned short i=0; i<=mRect->mVStrips; ++i) {
      glVertex3f(-0.5,v,0); glVertex3f(0.5,v,0);
      v += step;
    }
    glEnd();
  }

  glPopMatrix();
  glPopAttrib();
}

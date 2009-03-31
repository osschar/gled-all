// $#Header$
#include "Board_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

/**************************************************************************/

void Board_GL_Rnr::Render(RnrDriver* rd)
{
  Board& B = *mBoard;

  rd->GL()->Color(B.mColor);
  glNormal3f(0, 0, 1);

  if(B.mUDivs == 0 && B.mVDivs == 0) {
    const GLfloat u = B.mULen/2, v = B.mVLen/2;
    glBegin(GL_QUADS);
    glTexCoord2f(B.mTexX0, B.mTexY0);	glVertex3f(-u, -v, 0);
    glTexCoord2f(B.mTexX1, B.mTexY0);	glVertex3f( u, -v, 0);
    glTexCoord2f(B.mTexX1, B.mTexY1);	glVertex3f( u,  v, 0);
    glTexCoord2f(B.mTexX0, B.mTexY1);	glVertex3f(-u,  v, 0);
    glEnd();
  } else {
    const GLfloat ustep = B.mULen/(B.mUDivs + 1);
    const GLfloat vstep = B.mVLen/(B.mVDivs + 1);
    const GLfloat txstep = (B.mTexX1 - B.mTexX0)/(B.mUDivs + 1);
    const GLfloat tystep = (B.mTexY1 - B.mTexY0)/(B.mVDivs + 1);
    GLfloat u  = -B.mULen/2, tx  = B.mTexX0;
    const GLfloat v0 = -B.mVLen/2, ty0 = B.mTexY0;
    for(int i=0; i<=B.mUDivs; ++i) {
      const GLfloat un = u + ustep, txn = tx + txstep;
      GLfloat v = v0, ty = ty0;
      glBegin(GL_QUAD_STRIP);
      for(int j=0; j<=B.mVDivs; ++j) {
	glTexCoord2f(tx,  ty); glVertex3f(u,  v, 0);
	glTexCoord2f(txn, ty); glVertex3f(un, v, 0);
	v += vstep; ty += tystep;
      }
      v = -v0; ty = B.mTexY1;
      glTexCoord2f(tx,  ty); glVertex3f(u,  v, 0);
      glTexCoord2f(txn, ty); glVertex3f(un, v, 0);
      glEnd();
      u = un; tx = txn;
    }
  }

}

/**************************************************************************/

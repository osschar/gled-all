// $#Header$
#include "Board_GL_Rnr.h"

Board_GL_Rnr::~Board_GL_Rnr() {
  glDeleteTextures(1, &mTexture);
}

/**************************************************************************/

void Board_GL_Rnr::Draw(RnrDriver* rd)
{
  Board& B = *mBoard;

  glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT);

  if(B.bFillBack) glPolygonMode(GL_BACK, GL_FILL);

  // These are possible, but definitely not a reasonable default.
  // glShadeModel(GL_FLAT);
  // glDisable(GL_LIGHTING);

  const GLfloat u = B.mULen/2, v = B.mVLen/2;
  glColor4fv(B.mColor());
  glNormal3f(0,0,1);
  glBegin(GL_QUADS);
  // Texture y-coordinate is increasing in up->down direction.
  glTexCoord2f(B.mTexX0, B.mTexY1);	glVertex3f(-u, -v, 0);
  glTexCoord2f(B.mTexX1, B.mTexY1);	glVertex3f( u, -v, 0);
  glTexCoord2f(B.mTexX1, B.mTexY0);	glVertex3f( u,  v, 0);
  glTexCoord2f(B.mTexX0, B.mTexY0);	glVertex3f(-u,  v, 0);
  glEnd();

  glPopAttrib();
}

/**************************************************************************/

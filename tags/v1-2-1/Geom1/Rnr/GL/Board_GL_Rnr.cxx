// $#Header$
#include "Board_GL_Rnr.h"

Board_GL_Rnr::~Board_GL_Rnr() {
  glDeleteTextures(1, &mTexture);
}

/**************************************************************************/

void Board_GL_Rnr::Draw(RnrDriver* rd)
{
  glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT |
	       GL_POLYGON_BIT);

  glNormal3f(0,0,1); // Set normal b4 setting scale
  glPushMatrix();
  glScalef(mBoard->mULen, mBoard->mVLen, 1);

  if(mBoard->bFillBack) glPolygonMode(GL_BACK, GL_FILL);

  //glShadeModel(GL_FLAT);
  glColor4fv(mBoard->mColor());
  //glDisable(GL_LIGHTING);
  glBegin(GL_QUADS);
  glTexCoord2f(0,1);	glVertex3f(-0.5, -0.5, 0);
  glTexCoord2f(1,1);	glVertex3f( 0.5, -0.5, 0);
  glTexCoord2f(1,0);	glVertex3f( 0.5,  0.5, 0);
  glTexCoord2f(0,0);	glVertex3f(-0.5,  0.5, 0);
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

/**************************************************************************/

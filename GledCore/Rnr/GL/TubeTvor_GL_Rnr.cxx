// $#Header$

#include "TubeTvor_GL_Rnr.h"
#include <GL/gl.h>

/**************************************************************************/

void TubeTvor_GL_Rnr::TriangleFan(vRingInfo_i r1, vRingInfo_i r2)
{
  glBegin(GL_TRIANGLE_FAN); {
    if(r1->fNVert==1) {
      glArrayElement(r1->fIndex);
      Int_t M = r2->fIndex + r2->fNVert;
      for(int i=r2->fIndex; i<=M; i++) glArrayElement(i);
      //glArrayElement(r2->fIndex);
    } else {
      glArrayElement(r2->fIndex);
      Int_t M = r1->fIndex + r1->fNVert - 1;
      for(int i=M+1; i>=r1->fIndex; i--) glArrayElement(i);
      //glArrayElement(M);
    }
  } glEnd();
}

void TubeTvor_GL_Rnr::QuadStrip(vRingInfo_i r1, vRingInfo_i r2)
{
  glBegin(GL_QUAD_STRIP); {
    for(int j=0; j<=r1->fNVert; j++) {
      glArrayElement(r1->fIndex+j); glArrayElement(r2->fIndex+j);
    }
    // glArrayElement(r1->fIndex); glArrayElement(r2->fIndex);	
  } glEnd();
}

/**************************************************************************/

void TubeTvor_GL_Rnr::Render(TubeTvor* ttvor)
{
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glVertexPointer(3, GL_FLOAT, 0, ttvor->mV);
  glEnableClientState(GL_VERTEX_ARRAY);
  glNormalPointer(GL_FLOAT, 0, ttvor->mN);
  glEnableClientState(GL_NORMAL_ARRAY);
  if(ttvor->bColP) {
    glColorPointer(4, GL_FLOAT, 0, ttvor->mC);
    glEnableClientState(GL_COLOR_ARRAY);
  }
  if(ttvor->bTexP) {
    glTexCoordPointer(2, GL_FLOAT, 0, ttvor->mT);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  vRingInfo_i i = ttvor->mRings.begin();
  vRingInfo_i j = i + 1;
  while(j != ttvor->mRings.end()) {
    if(i->fNVert==1 || j->fNVert==1) {
      if(i->fDrawP && j->fDrawP) TriangleFan(i, j);
    }
    else if(i->fNVert == j->fNVert) {
      QuadStrip(i, j);
    }
    else {
      cout <<"TubeTvor_GL_Rnr ... can't render this monster ...\n";
      goto end;
    }
    i = j++;
  }
 end:
  glPopClientAttrib();
}

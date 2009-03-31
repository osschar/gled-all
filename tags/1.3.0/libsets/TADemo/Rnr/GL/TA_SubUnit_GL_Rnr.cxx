// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TA_SubUnit_GL_Rnr.h"
#include <Glasses/TA_Unit.h>

#include <GL/glew.h>

/**************************************************************************/

namespace {
  float _tex_square[4][4][2] = {
    { {0,0}, {1,0}, {1,1}, {0,1} },
    { {0,0}, {0,1}, {1,1}, {1,0} },
    { {1,1}, {1,0}, {0,0}, {0,1} },
    { {1,1}, {0,1}, {0,0}, {1,0} }
  };
}

void TA_SubUnit_GL_Rnr::Draw(RnrDriver* rd)
{
  v_TA_Prim_i beg = mTA_SubUnit->mPrims.begin();
  v_TA_Prim_i end = mTA_SubUnit->mPrims.end();
  if(mTA_SubUnit->mNPBeg > 0 && mTA_SubUnit->mNPBeg < mTA_SubUnit->mNPrim)
    beg += mTA_SubUnit->mNPBeg;
  if(mTA_SubUnit->mNPEnd > 0 && mTA_SubUnit->mNPEnd <= mTA_SubUnit->mNPrim)
    end = mTA_SubUnit->mPrims.begin() + mTA_SubUnit->mNPEnd;
  for(v_TA_Prim_i i=beg; i<end; ++i) {
    if(i->fN>=3) glNormal3fv(i->fNormal);
    if(i->fTATex==0 && i->fTextureName.Length() > 0 &&
       mTA_SubUnit->mUnit->GetTexCont())
      {
	i->fTATex = mTA_SubUnit->mUnit->GetTexCont()->FindTexture(i->fTextureName);
      }
    if(i->fTATex) i->fTATex->GL_Use();
    glBegin(GL_POLYGON);
    for(int j=0; j<i->fN; ++j) {
      glTexCoord2fv(_tex_square[mTA_SubUnit->mUnit->GetJ()][j]);
      glVertex3fv(&mTA_SubUnit->pVert[3*i->fIndices[j]]);
    }
    glEnd();
    if(i->fTATex) i->fTATex->GL_UnUse();
  }
}

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// TringTvor_GL_Rnr
//

#include "TringTvor_GL_Rnr.h"
#include <GL/gl.h>

void TringTvor_GL_Rnr::Render(TringTvor* ttvor)
{
  TringTvor& TT = *ttvor;

  if(TT.bSmoothShade) {

    //----------------
    // Smooth Shading
    //----------------

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glVertexPointer(3, GL_FLOAT, 0, TT.mVerts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, TT.mNorms);
    glEnableClientState(GL_NORMAL_ARRAY);
    if(TT.bColP) {
      glColorPointer(4, GL_UNSIGNED_BYTE, 0, TT.mCols);
      glEnableClientState(GL_COLOR_ARRAY);
    }
    if(TT.bTexP) {
      glTexCoordPointer(2, GL_FLOAT, 0, TT.mTexs);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if(TT.mStripEls != 0) {
      // Requires GL-1.4
      // glMultiDrawElements(GL_TRIANGLE_STRIP, TT.mStripLens,
      //                     GL_UNSIGNED_INT, TT.mStripBegs, TT.mNStrips);

      for(Int_t i=0; i<TT.mNStrips; ++i) {
	// glColor3f(0.2+0.8*colgen.Rndm(), 0.2+0.8*colgen.Rndm(), 0.2+0.8*colgen.Rndm());
	glDrawElements(GL_TRIANGLE_STRIP, TT.mStripLens[i],
		       GL_UNSIGNED_INT, TT.mStripBegs[i]);
      }

    } else {

      glDrawElements(GL_TRIANGLES, TT.mNTrings*3,
		     GL_UNSIGNED_INT, TT.mTrings);

    }

    glPopClientAttrib();

  } else {

    //--------------
    // Flat Shading
    //--------------

    GLint ex_shade_model;
    glGetIntegerv(GL_SHADE_MODEL, &ex_shade_model);
    glShadeModel(GL_FLAT);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glVertexPointer(3, GL_FLOAT, 0, TT.mVerts);
    glEnableClientState(GL_VERTEX_ARRAY);

    // TRandom colgen(2);

    if(TT.mStripEls) {

      for(Int_t i=0; i<TT.mNStrips; ++i) {
	glBegin(GL_TRIANGLE_STRIP);
	Int_t* idxp = TT.mStripBegs[i];

	glArrayElement(*(idxp++));
	glArrayElement(*(idxp++));
	
	Int_t* tring_idxp = TT.mStripTrings + (idxp - TT.mStripEls);
	Int_t  n          = TT.mStripLens[i] - 2;
	// glColor3f(0.2+0.8*colgen.Rndm(), 0.2+0.8*colgen.Rndm(), 0.2+0.8*colgen.Rndm());
	while(n-- > 0) {
	  glNormal3fv ( TT.TriangleNormal(*tring_idxp) );
	  if(TT.bColP) 
	    glColor4ubv( TT.TriangleColor(*tring_idxp) );
	  tring_idxp++;
	  glArrayElement(*(idxp++));
	}
	glEnd();
      }

    } else {
      
      glBegin(GL_TRIANGLES);
      Int_t*   T = TT.Triangle(0);
      Float_t* N = TT.TriangleNormal(0);
      UChar_t* C = TT.TriangleColor(0);
      for(Int_t t=0; t<TT.mNTrings; ++t) {
	glNormal3fv(N); N += 3;
	if(TT.bColP) { glColor4ubv(C);  C += 4; }
	glArrayElement(T[0]);
	glArrayElement(T[1]);
	glArrayElement(T[2]);
	T += 3;
      }
      glEnd();

    }

    glPopClientAttrib();
    glShadeModel(ex_shade_model);

  }
}

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// TringTvor_GL_Rnr
//

#include "TringTvor_GL_Rnr.h"
#include <GL/glew.h>

void TringTvor_GL_Rnr::Render(TringTvor* ttvor, Bool_t smoothp, Bool_t colp, Bool_t texp)
{
  if (smoothp)
    RenderSmooth(ttvor, colp, texp);
  else
    RenderFlat(ttvor, colp, texp);
}

/**************************************************************************/
/**************************************************************************/

void TringTvor_GL_Rnr::RenderSmooth(TringTvor* ttvor, Bool_t colp, Bool_t texp)
{
  TringTvor& TT = *ttvor;

  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glVertexPointer(3, GL_FLOAT, 0, TT.Verts());
  glEnableClientState(GL_VERTEX_ARRAY);
  glNormalPointer(GL_FLOAT, 0, TT.Norms());
  glEnableClientState(GL_NORMAL_ARRAY);
  if (colp && TT.HasCols())
  {
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, TT.Cols());
    glEnableClientState(GL_COLOR_ARRAY);
  }
  if (texp && TT.HasTexs())
  {
    glTexCoordPointer(2, GL_FLOAT, 0, TT.Texs());
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  if (TT.HasTrianlgeStrips())
  {
    // Requires GL-1.4.
    glMultiDrawElements(GL_TRIANGLE_STRIP, TT.mStripLens, GL_UNSIGNED_INT,
                        (const GLvoid**) TT.mStripBegs, TT.mNStrips);

    // for(Int_t i=0; i<TT.mNStrips; ++i) {
    //   glDrawElements(GL_TRIANGLE_STRIP, TT.mStripLens[i],
    //                 GL_UNSIGNED_INT, TT.mStripBegs[i]);
    // }
  }
  else
  {
    glDrawElements(GL_TRIANGLES, TT.mNTrings*3,
                   GL_UNSIGNED_INT, TT.Trings());
  }

  glPopClientAttrib();
}

void TringTvor_GL_Rnr::RenderFlat(TringTvor* ttvor, Bool_t colp, Bool_t texp)
{
  TringTvor& TT = *ttvor;

  GLint ex_shade_model;
  glGetIntegerv(GL_SHADE_MODEL, &ex_shade_model);
  glShadeModel(GL_FLAT);

  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glVertexPointer(3, GL_FLOAT, 0, TT.Verts());
  glEnableClientState(GL_VERTEX_ARRAY);
  if (colp && ! TT.HasTringCols())
    colp = false;
  if (texp && TT.HasTexs())
  {
    glTexCoordPointer(2, GL_FLOAT, 0, TT.Texs());
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  if (TT.HasTrianlgeStrips())
  {
    for (Int_t i=0; i<TT.mNStrips; ++i)
    {
      glBegin(GL_TRIANGLE_STRIP);
      Int_t* idxp = TT.mStripBegs[i];

      glArrayElement(*(idxp++));
      glArrayElement(*(idxp++));

      Int_t* tring_idxp = TT.mStripTrings + (idxp - TT.mStripEls);
      Int_t  n          = TT.mStripLens[i] - 2;
      while (n-- > 0)
      {
        glNormal3fv(TT.TriangleNormal(*tring_idxp));
        if (colp)
          glColor4ubv(TT.TriangleColor(*tring_idxp));
        ++tring_idxp;
        glArrayElement(*(idxp++));
      }
      glEnd();
    }
  }
  else
  {
    glBegin(GL_TRIANGLES);
    Int_t*   T = TT.Trings();
    Float_t* N = TT.TringNorms();
    UChar_t* C = TT.TringCols();
    for (Int_t t=0; t<TT.mNTrings; ++t)
    {
      glNormal3fv(N); N += 3;
      if (colp) { glColor4ubv(C);  C += 4; }
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

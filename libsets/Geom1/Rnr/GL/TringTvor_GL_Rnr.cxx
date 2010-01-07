// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringTvor_GL_Rnr.h"
#include <GL/glew.h>

#include <Rnr/GL/ZGlass_GL_Rnr.h>
#include <Rnr/GL/SphereTrings.h>

//______________________________________________________________________________
// TringTvor_GL_Rnr
//

TringTvor_GL_Rnr::TringTvor_GL_Rnr(TringTvor* tvor) :
  TT(*tvor)
{}

void TringTvor_GL_Rnr::BeginRender(Bool_t smooth_p, Bool_t tri_strips_p)
{
  bSmooth         = smooth_p;
  bTriangleStrips = tri_strips_p && TT.HasTrianlgeStrips();

  bColor = bTexture = false;

  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glVertexPointer(3, GL_FLOAT, 0, TT.Verts());
  glEnableClientState(GL_VERTEX_ARRAY);

  if (bSmooth)
  {
    glNormalPointer(GL_FLOAT, 0, TT.Norms());
    glEnableClientState(GL_NORMAL_ARRAY);
  }
}

void TringTvor_GL_Rnr::SetColorArray()
{
  if (bSmooth)
  {
    if (TT.HasCols())
      SetColorArray(TT.Cols());
  }
  else
  {
    if (TT.HasTringCols())
      SetColorArray(TT.TringCols());
  }
}

void TringTvor_GL_Rnr::SetColorArray(UChar_t* carr)
{
  mColorArray = carr;
  if (bSmooth)
  {
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, carr);
    glEnableClientState(GL_COLOR_ARRAY);
  }
  bColor = true;
}

void TringTvor_GL_Rnr::SetTextureArray()
{
  if (TT.HasTexs())
  {
    SetTextureArray(TT.Texs());
  }
}

void TringTvor_GL_Rnr::SetTextureArray(Float_t* tarr)
{
  glTexCoordPointer(2, GL_FLOAT, 0, tarr);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  bTexture = true;
}

void TringTvor_GL_Rnr::Render()
{
  if (bSmooth)
  {
    if (bTriangleStrips)
    {
      glMultiDrawElements(GL_TRIANGLE_STRIP, TT.mStripLens, GL_UNSIGNED_INT,
			  (const GLvoid**) TT.mStripBegs, TT.mNStrips);
    }
    else
    {
      glDrawElements(GL_TRIANGLES,  3*TT.mNTrings,
		     GL_UNSIGNED_INT, TT.Trings());
    }
  }
  else
  {
    if (bTriangleStrips)
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
	  if (bColor) glColor4ubv(Color(*tring_idxp));
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
      UChar_t* C = mColorArray;
      for (Int_t t=0; t<TT.mNTrings; ++t)
      {
	glNormal3fv(N); N += 3;
	if (bColor) { glColor4ubv(C);  C += 4; }
	glArrayElement(T[0]);
	glArrayElement(T[1]);
	glArrayElement(T[2]);
	T += 3;
      }
      glEnd();
    }
  }
}

void TringTvor_GL_Rnr::EndRender()
{
  glPopClientAttrib();
}


//==============================================================================
// Static, all inclusive method.
//==============================================================================

void TringTvor_GL_Rnr::Render(TringTvor* ttvor, Bool_t smoothp, Bool_t colp, Bool_t texp)
{
  GL_Enum_Holder shade(GL_SHADE_MODEL, smoothp ? GL_SMOOTH : GL_FLAT, glShadeModel);

  TringTvor_GL_Rnr rnr(ttvor);

  rnr.BeginRender(smoothp, true);

  if (colp) rnr.SetColorArray();
  if (texp) rnr.SetTextureArray();

  rnr.Render();

  rnr.EndRender();
}

//------------------------------------------------------------------------------

void TringTvor_GL_Rnr::RenderCEBBox(const Float_t* x, Float_t f, Bool_t pushpop)
{
  // Render center-extents axis-aligned bounding-box.
  // f is additional scale of the extents.

  f *= 2;
  if (pushpop) glPushMatrix();
  glTranslatef(x[0]-x[3], x[1]-x[4], x[2]-x[5]);
  glScalef(f*x[3], f*x[4], f*x[5]);
  SphereTrings::UnitFrameBox();
  if (pushpop) glPopMatrix();
}

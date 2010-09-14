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

typedef TringTvor TT;

TringTvor_GL_Rnr::TringTvor_GL_Rnr(TringTvor* tvor) :
  mTT(*tvor)
{}

void TringTvor_GL_Rnr::BeginSetup()
{
  // Clear internal state, prepare for new setup.

  mNMode = mCMode = mTMode = TT::M_None;
  mNArr  = 0;
  mCArr  = 0;
  mTArr  = 0;

  bTriangleStripsRequested = bTriangleStrips = false;
}

void TringTvor_GL_Rnr::SetupStandard(Bool_t color_p, Bool_t texture_p)
{
  // Set in accordance with default normal, color and texture modes in TringTvor.

  static const Exc_t _eh("TringTvor_GL_Rnr::SetupStandard ");

  SetNormalArray (mTT.mNormalMode);
  SetColorArray  (mTT.mColorMode);
  SetTextureArray(mTT.mTextureMode);
}

void TringTvor_GL_Rnr::SetupStandardSmooth(Bool_t color_p, Bool_t texture_p)
{
  // Set for smooth rendering -- only existing arrays from TringTvor are used.

  static const Exc_t _eh("TringTvor_GL_Rnr::SetupStandardSmooth ");

  if (mTT.HasNorms())
  {
    mNMode = TT::M_PerVertex;
    mNArr  = mTT.Norms();
  }
  else
  {
    throw _eh + "vertex normal array does not exist.";
  }

  if (color_p && mTT.HasCols())
  {
    mCMode = TT::M_PerVertex;
    mCArr  = mTT.Cols();
  }
  if (mTT.HasTexs())
  {
    mTMode = TT::M_PerVertex;
    mTArr  = mTT.Texs();
  }
}

void TringTvor_GL_Rnr::SetupStandardFlat(Bool_t color_p, Bool_t texture_p)
{
  // Set for flat rendering -- only existing arrays from TringTvor are used.

  static const Exc_t _eh("TringTvor_GL_Rnr::SetupStandardFlat ");

  if (mTT.HasTringNorms())
  {
    mNMode = TT::M_PerTriangle;
    mNArr  = mTT.TringNorms();
  }
  else
  {
    throw _eh + "triangle normal array does not exist.";
  }

  if (color_p && mTT.HasTringCols())
  {
    mCMode = TT::M_PerTriangle;
    mCArr  = mTT.TringCols();
  }
  if (mTT.HasTexs())
  {
    mTMode = TT::M_PerVertex;
    mTArr  = mTT.Texs();
  }
}

void TringTvor_GL_Rnr::SetNormalArray(TringTvor::Mode_e mode, Float_t* narr)
{
  static const Exc_t _eh("TringTvor_GL_Rnr::SetNormalArray ");

  if (mode == TT::M_PerVertex)
  {
    if (narr == 0)
    {
      if (! mTT.HasNorms())
	throw _eh + "No vertex normals.";
      narr = mTT.Norms();
    }
    mNMode = mode;
    mNArr  = narr;
  }
  else if (mode == TT::M_PerTriangle)
  {
    if (narr == 0)
    {
      if (! mTT.HasTringNorms())
	throw _eh + "No triangle normals.";
      narr = mTT.TringNorms();
    }
    mNMode = mode;
    mNArr  = narr;
  }
}

void TringTvor_GL_Rnr::SetColorArray(TringTvor::Mode_e mode, UChar_t* carr)
{
  if (carr == 0)
  {
    if      (mode == TT::M_PerVertex)   carr = mTT.Cols();
    else if (mode == TT::M_PerTriangle) carr = mTT.TringCols();
  }
  if (carr != 0)
  {
    mCMode = mode;
    mCArr  = carr;
  }
}

void TringTvor_GL_Rnr::SetTextureArray(TringTvor::Mode_e mode, Float_t* tarr)
{
  if (tarr == 0)
  {
    if      (mode == TT::M_PerVertex)   tarr = mTT.Texs();
    else if (mode == TT::M_PerTriangle) tarr = mTT.TringTexs();
  }
  if (tarr != 0)
  {
    mTMode = mode;
    mTArr  = tarr;
  }
}

void TringTvor_GL_Rnr::RequestTriangleStrips()
{
  bTriangleStripsRequested = true;
}

void TringTvor_GL_Rnr::EndSetup()
{
  // Check requested setup sanity, set rendering mode.

  static const Exc_t _eh("TringTvor_GL_Rnr::EndSetup ");

  // Check smooth / flat / mixed mode.
  if ((mNMode == TT::M_PerVertex) &&
      (mCMode == TT::M_PerVertex || mCMode == TT::M_None) &&
      (mTMode == TT::M_PerVertex || mTMode == TT::M_None))
  {
    mRenderMode = RM_Smooth;
    if (bTriangleStripsRequested && mTT.HasTrianlgeStrips())
      bTriangleStrips = true;
  }
  else if ((mNMode == TT::M_PerTriangle) &&
	   (mCMode == TT::M_PerTriangle || mCMode == TT::M_None) &&
	   (mTMode == TT::M_PerVertex   || mTMode == TT::M_None))
  {
    // Flat
    mRenderMode = RM_Flat;
    if (bTriangleStripsRequested && mTT.HasTrianlgeStrips())
      bTriangleStrips = true;
  }
  else
  {
    mRenderMode = RM_Mixed;
    if (bTriangleStripsRequested)
      throw _eh + "Triangle-strips requested -- not possible in mixed mode.";
  }
}

//==============================================================================

void TringTvor_GL_Rnr::BeginRender()
{
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glVertexPointer(3, GL_FLOAT, 0, mTT.Verts());
  glEnableClientState(GL_VERTEX_ARRAY);

  if (mNMode == TT::M_PerVertex)
  {
    glNormalPointer(GL_FLOAT, 0, mNArr);
    glEnableClientState(GL_NORMAL_ARRAY);
  }
  if (mCMode == TT::M_PerVertex)
  {
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, mCArr);
    glEnableClientState(GL_COLOR_ARRAY);
  }
  if (mTMode == TT::M_PerVertex)
  {
    glTexCoordPointer(2, GL_FLOAT, 0, mTArr);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }
}

void TringTvor_GL_Rnr::Render()
{
  static const Exc_t _eh("TringTvor_GL_Rnr::Render ");

  switch (mRenderMode)
  {
    case RM_Smooth: RenderSmooth(); break;
    case RM_Flat:   RenderFlat();   break;
    case RM_Mixed:  RenderMixed();  break;
    default:        throw _eh + "Bad render mode.";
  }
}

void TringTvor_GL_Rnr::RenderSmooth()
{
  if (bTriangleStrips)
  {
    glMultiDrawElements(GL_TRIANGLE_STRIP, mTT.mStripLens, GL_UNSIGNED_INT,
			(const GLvoid**) mTT.mStripBegs, mTT.mNStrips);
  }
  else
  {
    glDrawElements(GL_TRIANGLES,  3*mTT.mNTrings,
		   GL_UNSIGNED_INT, mTT.Trings());
  }
}

void TringTvor_GL_Rnr::RenderFlat()
{
  const Bool_t col_p = mCMode == TT::M_PerTriangle;

  if (bTriangleStrips)
  {
    for (Int_t i=0; i<mTT.mNStrips; ++i)
    {
      glBegin(GL_TRIANGLE_STRIP);
      Int_t* idxp = mTT.mStripBegs[i];

      glArrayElement(*(idxp++));
      glArrayElement(*(idxp++));

      Int_t* tring_idxp = mTT.mStripTrings + (idxp - mTT.mStripEls);
      Int_t  n          = mTT.mStripLens[i] - 2;
      while (n-- > 0)
      {
	glNormal3fv(mTT.TriangleNormal(*tring_idxp));
	if (col_p) glColor4ubv(TriangleColor(*tring_idxp));
	++tring_idxp;
	glArrayElement(*(idxp++));
      }
      glEnd();
    }
  }
  else
  {
    glBegin(GL_TRIANGLES);
    Int_t*   T = mTT.Trings();
    Float_t* N = mNArr;
    UChar_t* C = mCArr;
    for (Int_t t=0; t<mTT.mNTrings; ++t)
    {
      glNormal3fv(N); N += 3;
      if (col_p) { glColor4ubv(C);  C += 4; }
      glArrayElement(T[0]);
      glArrayElement(T[1]);
      glArrayElement(T[2]);
      T += 3;
    }
    glEnd();
  }
}

void TringTvor_GL_Rnr::RenderMixed()
{
  const Bool_t norm_p = mNMode == TT::M_PerTriangle;
  const Bool_t col_p  = mCMode == TT::M_PerTriangle;
  const Bool_t tex_p  = mTMode == TT::M_PerTriangle;

  Float_t* na = mNArr;
  UChar_t* ca = mCArr;
  Float_t* ta = mTArr; 

  glBegin(GL_TRIANGLES);
  Int_t* T = mTT.Trings();

  for (Int_t t = 0; t < mTT.mNTrings; ++t)
  {
    if (norm_p) { glNormal3fv(na); na += 3; }
    if (col_p)  { glColor4ubv(ca); ca += 4; }
    if (tex_p)  
    {
      glTexCoord2fv(ta); ta += 2;
      glArrayElement(T[0]);
      glTexCoord2fv(ta); ta += 2;
      glArrayElement(T[1]);
      glTexCoord2fv(ta); ta += 2;
      glArrayElement(T[2]);
    }
    else
    {
      glArrayElement(T[0]);
      glArrayElement(T[1]);
      glArrayElement(T[2]);
    }
    T += 3;
  }
  glEnd();
}

void TringTvor_GL_Rnr::EndRender()
{
  glPopClientAttrib();
}


//==============================================================================
// Static, all inclusive method.
//==============================================================================

void TringTvor_GL_Rnr::Render(TringTvor* ttvor)
{
  TringTvor_GL_Rnr rnr(ttvor);

  rnr.BeginSetup();
  rnr.SetupStandard();
  rnr.EndSetup();

  GL_Enum_Holder shade(GL_SHADE_MODEL, rnr.mRenderMode == RM_Flat ? GL_FLAT : GL_SMOOTH, glShadeModel);

  rnr.BeginRender();
  rnr.Render();
  rnr.EndRender();
}

void TringTvor_GL_Rnr::Render(TringTvor* ttvor, Bool_t smoothp, Bool_t colp, Bool_t texp)
{
  TringTvor_GL_Rnr rnr(ttvor);

  rnr.BeginSetup();
  if (smoothp)
    rnr.SetupStandardSmooth(colp, texp);
  else
    rnr.SetupStandardFlat(colp, texp);
  rnr.EndSetup();

  GL_Enum_Holder shade(GL_SHADE_MODEL, smoothp ? GL_SMOOTH : GL_FLAT, glShadeModel);

  rnr.BeginRender();
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

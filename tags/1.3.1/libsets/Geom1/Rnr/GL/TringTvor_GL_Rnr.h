// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_TringTvor_GL_Rnr_H
#define Geom1_TringTvor_GL_Rnr_H

#include <Stones/TringTvor.h>

class TringTvor_GL_Rnr
{
protected:
  TringTvor   &TT;

  Bool_t       bSmooth;
  Bool_t       bTriangleStrips;

  Bool_t       bColor;
  Bool_t       bTexture;

  UChar_t     *mColorArray;

  UChar_t* Color(Int_t i) const { return &(mColorArray[4*i]); }

public:
  TringTvor_GL_Rnr(TringTvor* tvor);

  void BeginRender(Bool_t smooth_p, Bool_t tri_strips_p);

  void SetColorArray();
  void SetColorArray(UChar_t* carr);

  void SetTextureArray();
  void SetTextureArray(Float_t* tarr);

  void Render();

  void EndRender();


  // Static, all inclusive method.

  static void Render(TringTvor* ttvor, Bool_t smoothp, Bool_t colp=true, Bool_t texp=true);

  static void RenderCEBBox(const Float_t* x, Float_t f=1.0f, Bool_t pushpop=true);

}; // endclass TringTvor_GL_Rnr

#endif

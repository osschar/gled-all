// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_TringTvor_GL_Rnr_H
#define Geom1_TringTvor_GL_Rnr_H

#include <Stones/TringTvor.h>

class TringTvor_GL_Rnr {
public:
  static void Render(TringTvor* ttvor, Bool_t smoothp, Bool_t colp=true, Bool_t texp=true);

  static void RenderSmooth(TringTvor* ttvor, Bool_t colp=true, Bool_t texp=true);
  static void RenderFlat(TringTvor* ttvor, Bool_t colp=true, Bool_t texp=true);
}; // endclass TringTvor_GL_Rnr

#endif

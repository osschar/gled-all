// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SEvTaskState_GL_RNR_H
#define GledCore_SEvTaskState_GL_RNR_H

#include <Stones/SEvTaskState.h>
class ZColor;

class SEvTaskState_GL_Rnr {

public:
  static void RenderHisto(const SEvTaskState& ts, ZColor* cols);
  static void RenderBar(const SEvTaskState& ts, ZColor* cols);
  
}; // endclass SEvTaskState_GL_Rnr

#endif

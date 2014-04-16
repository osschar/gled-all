// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTS_H
#define GTS_GTS_H

#ifndef __CINT__

class ZTrans;

namespace GTS
{

#include <gts.h>

  GtsSurface* MakeDefaultSurface();

  void InvertSurface(GtsSurface* s);

  void TransformSurfaceVertices(GtsSurface* s, ZTrans* t);
  void RotateSurfaceVertices(GtsSurface* s, ZTrans* t);

  void WriteSurfaceToFile(GtsSurface* s, const TString& file);
}

#endif

#endif

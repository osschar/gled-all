// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTS.h"

using namespace GTS;

GtsSurface* GTS::MakeDefaultSurface()
{
  return gts_surface_new(gts_surface_class (), gts_face_class (),
			 gts_edge_class (),    gts_vertex_class ());
}

//==============================================================================

namespace
{
  int face_inverter(GtsFace* f, int* dum)
  {
    GtsEdge* egg = f->triangle.e1;
    f->triangle.e1 = f->triangle.e2;
    f->triangle.e2 = egg;
    return 0;
  }
}

void GTS::InvertSurface(GtsSurface* s)
{
  gts_surface_foreach_face(s, (GtsFunc)face_inverter, 0);
}

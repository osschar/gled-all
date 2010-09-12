// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTS.h"
#include "Stones/ZTrans.h"

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

//==============================================================================

namespace
{
  void vertex_transformer(GtsVertex* v, ZTrans* t)
  {
    t->MultiplyVec3IP(&v->p.x, 1);
  }
}

void GTS::TransformSurfaceVertices(GtsSurface* s, ZTrans* t)
{
  gts_surface_foreach_vertex(s, (GtsFunc) vertex_transformer, t);
}

//==============================================================================

void GTS::WriteSurfaceToFile(GtsSurface* s, const TString& file)
{
  FILE* fp = fopen(file, "w");
  if (!fp) {
    ISerr(GForm("GTS::WriteSurfaceToFile Can not open file '%s'.", file.Data()));
    return;
  }
  gts_surface_write(s, fp);
  fclose(fp);
}

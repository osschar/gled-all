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

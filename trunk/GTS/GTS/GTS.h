// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTS_H
#define GTS_GTS_H

#include <TString.h>

#ifndef __CINT__

namespace GTS {

#include <gts.h>

  class Surface {
  public:
    GtsSurface* surface;
    
    Surface() : surface(0) {}
    ~Surface();

    static Surface* Load(const TString& grl);

    void Replace(GtsSurface* new_surf);

    void Invert();
  };

}

#endif

#endif

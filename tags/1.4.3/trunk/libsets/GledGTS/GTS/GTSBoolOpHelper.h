// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTSBoolOpHelper_H
#define GTS_GTSBoolOpHelper_H

#ifndef __CINT__

#include "Gled/GledTypes.h"
#include "GTS.h"

class GTSurf;

namespace GTS
{
  struct BoolOpHelperImpl;

  class BoolOpHelper
  {
    GTSurf          *target;
    GtsSurface      *a_surf;
    GtsSurface      *b_surf;
    GtsSurfaceInter *inter;
    GtsSurface      *result;
    double           eps_a, eps_p, eps_l;

    int              debug; // not used yet

    auto_ptr<BoolOpHelperImpl> impl;

    // ----------------------------------------------------------------
    // epsi triangles -- area < eps_a, perimeter < eps_p

    bool is_epsi(GtsTriangle *t);

    void collapse_adjacent_epsi_triangles();

    static void epsi_select     (GtsFace* f, BoolOpHelper* boh);
    static void epsi_pair_select(GtsFace* f, BoolOpHelper* boh);

    // ----------------------------------------------------------------
    // zeta - triangles -- area < eps_a, perimeter >= eps_p

    bool is_zeta(GtsTriangle *t);

    void handle_zeta_triangles();

    static void zeta_select(GtsFace* f, BoolOpHelper* boh);

  public:

    BoolOpHelper(GTSurf* tgt, GTSurf* a, GTSurf* b, const Exc_t& _eh);
    ~BoolOpHelper();

    void set_debug(int d) { debug = d; }
    
    void BuildInter(const Exc_t& _eh);

    void PostProcess();

    void MakeMerge();
    void MakeUnion();
    void MakeIntersection();
    void MakeDifference();

    GtsSurface* TakeResult();
  };

}

#endif

#endif

// $Id$

#ifndef Geom1_TubeTvor_GL_Rnr_H
#define Geom1_TubeTvor_GL_Rnr_H

#include <Stones/TubeTvor.h>

class TubeTvor_GL_Rnr {
public:
  static void TriangleFan(TubeTvor::vRingInfo_i p, TubeTvor::vRingInfo_i r);
  static void QuadStrip(TubeTvor::vRingInfo_i r1, TubeTvor::vRingInfo_i r2);

  static void Render(TubeTvor* ttvor);
};

#endif

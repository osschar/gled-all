// $Header$

#ifndef GledCore_TubeTvor_GL_Rnr_H
#define GledCore_TubeTvor_GL_Rnr_H

#include <Tvor/TubeTvor.h>

class TubeTvor_GL_Rnr {
public:
  static void TriangleFan(vRingInfo_i p, vRingInfo_i r);
  static void QuadStrip(vRingInfo_i r1, vRingInfo_i r2);

  static void Render(TubeTvor* ttvor);
};

#endif

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_ParaSurf_H
#define Var1_ParaSurf_H

#include <Glasses/ZGlass.h>

class TriMesh;

namespace Opcode
{
class Plane;
class Point;
class Ray;
}

class TRandom;


class ParaSurf : public ZGlass
{
  MAC_RNR_FRIENDS(ParaSurf);

private:
  void _init();

protected:
  Float_t  mMinF;   // X{G} 7 ValOut(-join=>1)
  Float_t  mMaxF;   // X{G} 7 ValOut()
  Float_t  mMinG;   // X{G} 7 ValOut(-join=>1)
  Float_t  mMaxG;   // X{G} 7 ValOut()
  Float_t  mMinH;   // X{G} 7 ValOut(-join=>1)
  Float_t  mMaxH;   // X{G} 7 ValOut()
  Float_t  mMeanH;  // X{G} 7 ValOut(-join=>1)
  Float_t  mSigmaH; // X{G} 7 ValOut()

  Float_t  mEpsilon;

  static const Float_t  sEpsilonFac;
  static const Float_t  sPi, sTwoPi;

public:
  ParaSurf(const Text_t* n="ParaSurf", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }
  virtual ~ParaSurf() {}

  void FindMinMaxFGH(TriMesh* mesh);  // X{E} C{1} 7 MCWButt()
  void FindMinMaxH  (TriMesh* mesh);  // X{E} C{1} 7 MCWButt()
  void RandomizeH(TriMesh* mesh,
                  Float_t  fg_chr_len, Float_t h_chr_len,
                  Float_t  alpha=0.8,  Bool_t  accum_dh=false,
                  Float_t  n_pass=1); // X{E} C{1} 7 MCWButt()

  Float_t GetDeltaH() const { return mMaxH - mMinH; }

  virtual void SetupEdgePlanes() {}

  // !!! some/most of those should be abstract

  virtual void originpos(Float_t* x);

  virtual void pos2fgh(const Float_t* x, Float_t* f) = 0;
  virtual void fgh2pos(const Float_t* f, Float_t* x) = 0;

  virtual void fgh2fdir(const Float_t* f, Float_t* d) = 0;
  virtual void fgh2gdir(const Float_t* f, Float_t* d) = 0;
  virtual void fgh2hdir(const Float_t* f, Float_t* d) = 0;
  // for possible optimizations
  virtual void    pos2hdir(const Float_t* x, Float_t* d)     = 0;
  virtual Float_t pos2hray(const Float_t* x, Opcode::Ray& r) = 0;
  virtual void    pos2fghdir(const Float_t* x,
                             Float_t* fdir, Float_t* gdir, Float_t* hdir);

  // Subtract fgh values, taking care of U(1) variables (like angles).
  virtual void sub_fgh(Float_t* a, Float_t* b, Float_t* delta);

  // These two return a point ABOVE the surface (at maxH + eps)
  virtual void random_fgh(TRandom& rnd, Float_t* f) = 0;
  virtual void random_pos(TRandom& rnd, Float_t* x);

  virtual Int_t          n_edge_planes() { return 0; }
  virtual Opcode::Plane* edge_planes()   { return 0; }

  virtual Bool_t  support_wrap()         { return false; }
  virtual void    wrap(Float_t* x, Int_t plane, Float_t dist) {}

  // U(1) wrap-around foos
  static void    U1Wrap(Float_t& x)          { if (x > sPi) x -= sTwoPi; else if (x < -sPi) x += sTwoPi; }
  static Float_t U1Add(Float_t a, Float_t b) { Float_t x = a + b; U1Wrap(x); return x; }
  static Float_t U1Sub(Float_t a, Float_t b) { Float_t x = a - b; U1Wrap(x); return x; }

#include "ParaSurf.h7"
  ClassDef(ParaSurf, 1)
}; // endclass ParaSurf


#endif

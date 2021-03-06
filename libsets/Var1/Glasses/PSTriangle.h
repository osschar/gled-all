// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_PSTriangle_H
#define Var1_PSTriangle_H

#include <Glasses/ParaSurf.h>

class PSTriangle : public ParaSurf
{
  MAC_RNR_FRIENDS(PSTriangle);

private:
  void _init();

protected:
  Float_t  mX0; // X{GS} 7 Value(-join=>1)
  Float_t  mY0; // X{GS} 7 Value()
  Float_t  mX1; // X{GS} 7 Value(-join=>1)
  Float_t  mY1; // X{GS} 7 Value()
  Float_t  mX2; // X{GS} 7 Value(-join=>1)
  Float_t  mY2; // X{GS} 7 Value()

  Opcode::Plane*   mEdgePlanes; //!

public:
  PSTriangle(const Text_t* n="PSTriangle", const Text_t* t=0);
  virtual ~PSTriangle();

  void Scale(Float_t s);

  virtual void SetupEdgePlanes();

  virtual Float_t Surface();

  virtual void origin_fgh(Float_t* g);
  virtual void origin_pos(Float_t* x);

  virtual void pos2fgh(const Float_t* x, Float_t* f);
  virtual void fgh2pos(const Float_t* f, Float_t* x);

  virtual void fgh2fdir(const Float_t* f, Float_t* d);
  virtual void fgh2gdir(const Float_t* f, Float_t* d);
  virtual void fgh2hdir(const Float_t* f, Float_t* d);

  virtual void    pos2hdir(const Float_t* x, Float_t* d);
  virtual Float_t pos2hray(const Float_t* x, Opcode::Ray& r);

  virtual void    pos2grav(const Float_t* x, GravData& gd);

  virtual void random_fgh(TRandom& rnd, Float_t* f);
  virtual void random_pos(TRandom& rnd, Float_t* x);

  virtual Int_t          n_edge_planes() { return 3; }
  virtual Opcode::Plane* edge_planes()   { return mEdgePlanes; }

#include "PSTriangle.h7"
  ClassDef(PSTriangle, 1);
}; // endclass PSTriangle


#endif

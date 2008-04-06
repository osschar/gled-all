// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_PSRectangle_H
#define Var1_PSRectangle_H

#include <Glasses/ParaSurf.h>

class PSRectangle : public ParaSurf
{
  MAC_RNR_FRIENDS(PSRectangle);

private:
  void _init();

protected:
  Float_t          mMinX; // X{GS} 7 Value(-join=>1)
  Float_t          mMaxX; // X{GS} 7 Value()
  Float_t          mMinY; // X{GS} 7 Value(-join=>1)
  Float_t          mMaxY; // X{GS} 7 Value()

  Opcode::Plane*   mEdgePlanes; //!

public:
  PSRectangle(const Text_t* n="PSRectangle", const Text_t* t=0);
  virtual ~PSRectangle();

  virtual void SetupEdgePlanes();

  virtual Float_t surface();

  virtual void origin_fgh(Float_t* g);
  virtual void origin_pos(Float_t* x);

  virtual void pos2fgh(const Float_t* x, Float_t* f);
  virtual void fgh2pos(const Float_t* f, Float_t* x);

  virtual void fgh2fdir(const Float_t* f, Float_t* d);
  virtual void fgh2gdir(const Float_t* f, Float_t* d);
  virtual void fgh2hdir(const Float_t* f, Float_t* d);

  virtual void    pos2hdir(const Float_t* x, Float_t* d);
  virtual Float_t pos2hray(const Float_t* x, Opcode::Ray& r);

  virtual void random_fgh(TRandom& rnd, Float_t* f);
  virtual void random_pos(TRandom& rnd, Float_t* x);

  virtual Int_t          n_edge_planes() { return 4; }
  virtual Opcode::Plane* edge_planes()   { return mEdgePlanes; }

  virtual Bool_t  support_wrap()  { return true; }
  virtual void    wrap(Float_t* x, Int_t plane, Float_t dist);

#include "PSRectangle.h7"
  ClassDef(PSRectangle, 1)
}; // endclass PSRectangle


#endif

// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_PSTorus_H
#define Var1_PSTorus_H

#include <Glasses/ParaSurf.h>

class PSTorus : public ParaSurf
{
  MAC_RNR_FRIENDS(PSTorus);

private:
  void _init();

protected:
  Bool_t  bInside;    // X{GS} 7 Bool()
  Float_t mRM;        // X{GS} 7 Value(-range=>[0.001,1000, 1,1000], -join=>1)
  Float_t mRm;        // X{GS} 7 Value(-range=>[0.001,1000, 1,1000])

public:
  PSTorus(const Text_t* n="PSTorus", const Text_t* t=0) :
    ParaSurf(n,t) { _init(); }

  virtual Float_t surface();

  virtual void pos2fgh(const Float_t* x, Float_t* f);
  virtual void fgh2pos(const Float_t* f, Float_t* x);

  virtual void fgh2fdir(const Float_t* f, Float_t* d);
  virtual void fgh2gdir(const Float_t* f, Float_t* d);
  virtual void fgh2hdir(const Float_t* f, Float_t* d);

  virtual void    pos2hdir(const Float_t* x, Float_t* d);
  virtual Float_t pos2hray(const Float_t* x, Opcode::Ray& r);

  // Subtract fgh values, taking care of U(1) variables (like angles).
  virtual void sub_fgh(Float_t* a, Float_t* b, Float_t* delta);

  // Put fgh values into regular intervals.
  virtual void regularize_fg(Float_t* f);

  virtual void random_fgh(TRandom& rnd, Float_t* f);
  // virtual void random_pos(TRandom& rnd, Float_t* x);

#include "PSTorus.h7"
  ClassDef(PSTorus, 1)
}; // endclass PSTorus


#endif

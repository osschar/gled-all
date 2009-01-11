// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_GravData_H
#define Var1_GravData_H

#include <Gled/GledTypes.h>

#include <Opcode/Opcode.h>

#include <TObject.h>

class GravData : public TObject
{
public:
  typedef list<pair<Float_t, GravData*> > lGravFraction_t;
  typedef lGravFraction_t::iterator       lGravFraction_i;

protected:
  void clear();
  void add(Float_t f, GravData& gd);

public:
  Float_t fPos[3];
  Float_t fDir[3];
  Float_t fMag;
  Float_t fLDer;    // Derivative in longitudinal direction (direction of gravity).
  Float_t fTDer;    // Derivative in tangential plane.
  Float_t fH;       // Height as defined by the surface.
  Float_t fDown[3]; // Parametric down direction.

  // Storage for users of the struct, like Dynamico's.
  Float_t fSafeTime;     //!
  Float_t fSafeDistance; //!

  GravData() : fSafeTime(0), fSafeDistance(0) {}
  virtual ~GravData() {}

  Opcode::Point& Pos()  { return * (Opcode::Point*) fPos;  }
  Opcode::Point& Dir()  { return * (Opcode::Point*) fDir;  }
  Opcode::Point& Down() { return * (Opcode::Point*) fDown; }

  Bool_t DecaySafeties(Float_t dt, Float_t dl)
  { fSafeTime -= dt; fSafeDistance -= dl; return fSafeTime < 0 || fSafeDistance < 0; }

  void Combine(lGravFraction_t& fractions);

  virtual void Print(Option_t *option="") const;

#include "GravData.h7"
  ClassDef(GravData, 1);
}; // endclass GravData

#endif

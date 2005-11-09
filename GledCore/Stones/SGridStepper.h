// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SGridStepper_H
#define GledCore_SGridStepper_H

#include <Rtypes.h>
class ZNode;

class SGridStepper {
  Int_t *ls[3], *ns[3];
public:
  enum StepMode_e { SM_XYZ, SM_YXZ, SM_XZY };
  StepMode_e Mode; 

  Int_t   nx, ny, nz;
  Int_t   Nx, Ny, Nz;
  Float_t Dx, Dy, Dz;
  Float_t Ox, Oy, Oz;

  SGridStepper(Int_t sm);

  void Reset();
  void Subtract(SGridStepper& s);
  void SetNs(Int_t nx, Int_t ny, Int_t nz=1)
  { Nx = nx; Ny = ny; Nz = nz; }
  void SetDs(Float_t dx, Float_t dy, Float_t dz=0)
  { Dx = dx; Dy = dy; Dz = dz; }
  
  bool Step();

  void GetPosition(Float_t* p);
  void SetNode(ZNode* node);
  void SetNodeAdvance(ZNode* node);

  ClassDef(SGridStepper, 1)
}; // endclass SGridStepper

#endif

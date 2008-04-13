// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SGridStepper.h"
#include <Glasses/ZNode.h>

//______________________________________________________________________
// SGridStepper
//

ClassImp(SGridStepper);

SGridStepper::SGridStepper(Int_t sm) : Mode(StepMode_e(sm))
{
  switch(Mode) {
  default:
  case SM_XYZ:
    ls[0] = &Nx; ls[1] = &Ny; ls[2] = &Nz;
    ns[0] = &nx; ns[1] = &ny; ns[2] = &nz;
    break;
  case SM_YXZ:
    ls[0] = &Ny; ls[1] = &Nx; ls[2] = &Nz;
    ns[0] = &ny; ns[1] = &nx; ns[2] = &nz;
    break;
  case SM_XZY:
    ls[0] = &Nx; ls[1] = &Nz; ls[2] = &Ny;
    ns[0] = &nx; ns[1] = &nz; ns[2] = &ny;
    break;
  }

  nx = ny = nz = 0;
  Nx = Ny = Nz = 16;
  Dx = Dy = Dz = 1;
  Ox = Oy = Oz = 0;
}

void SGridStepper::Reset()
{
  nx = ny = nz = 0;
}

void SGridStepper::Subtract(SGridStepper& s)
{
  Ox = -(s.Ox + s.nx*s.Dx);
  Oy = -(s.Oy + s.ny*s.Dy);
  Oz = -(s.Oz + s.nz*s.Dz);
}
/**************************************************************************/

bool SGridStepper::Step()
{
  (*ns[0])++;
  if(*ns[0] >= *ls[0]) {
    *ns[0] = 0; (*ns[1])++;
    if(*ns[1] >= *ls[1]) {
      *ns[1] = 0; (*ns[2])++;
      if(*ns[2] >= *ls[2]) {
	return false;
      }
    }
  }
  return true;
}

/**************************************************************************/

void SGridStepper::GetPosition(Float_t* p)
{
  p[0] = Ox + nx*Dx; p[1] = Oy + ny*Dy; p[2] = Oz + nz*Dz;
}

void SGridStepper::SetNode(ZNode* node)
{
  node->SetPos(Ox + nx*Dx, Oy + ny*Dy, Oz + nz*Dz);
}

void SGridStepper::SetNodeAdvance(ZNode* node)
{
  SetNode(node);
  Step();
}

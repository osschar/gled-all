// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// FormulaMover
//
//

#include "FormulaMover.h"
#include <TFormula.h>
#include "FormulaMover.c7"

ClassImp(FormulaMover);

/**************************************************************************/

void FormulaMover::_init()
{
  mForX = mForY = mForZ = 0;
  mForPhi = mForTheta = mForEta = 0;
}

/**************************************************************************/

void FormulaMover::EmitPosChangedRay()
{
  static const Exc_t _eh("FormulaMover::EmitPosChangedRay ");
  cout << _eh <<endl;
}

void FormulaMover::EmitRotChangedRay()
{
  static const Exc_t _eh("FormulaMover::EmitRotChangedRay ");
  cout << _eh <<endl;
}

/**************************************************************************/

void FormulaMover::TimeTick(Double_t t, Double_t dt)
{
  printf("FormulaMover::TimeTick t=%lf, ft=%lf\n", t, dt);
}

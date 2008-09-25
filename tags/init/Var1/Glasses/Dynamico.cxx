// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Dynamico
//
//

#include "Dynamico.h"
#include <Glasses/TriMesh.h>
#include "Dynamico.c7"

#include <Opcode/Opcode.h>

ClassImp(Dynamico)

/**************************************************************************/

void Dynamico::_init()
{
  mTrans.SetPos(0.5, 0.5, 8);
  Double_t oos2 = 1.0/TMath::Sqrt(2);
  mTrans.SetBaseVec(1,  oos2, oos2, 0);
  mTrans.SetBaseVec(2,  0,    0,    1);

  mV = mW = 0;
  mLevH = 0.1;

  bRnrSelf = true;

  mOPCRCCache = OPC_INVALID_ID;

}

/**************************************************************************/


/**************************************************************************/

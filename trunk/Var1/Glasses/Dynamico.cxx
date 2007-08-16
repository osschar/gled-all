// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Dynamico
//
//

#include "Dynamico.h"
#include "Dynamico.c7"

#include <Stones/TringTvor.h>

#include <TMath.h>

ClassImp(Dynamico);

/**************************************************************************/

void Dynamico::_init()
{
  mTrans.SetPos(0.5, 0.5, 8.0);
  Float_t oos2 = 1.0/TMath::Sqrt(2);
  mTrans.SetBaseVec(1,  oos2, oos2, 0);
  mTrans.SetBaseVec(2,  0, 0, 1);
  mTrans.SetBaseVecViaCross(3);

  bAABBok = false;

  mV = mW = 0.0f;
  mLevH   = 0.1f;

  bRnrSelf = true;

  mOPCRCCache = OPC_INVALID_ID;

}

/**************************************************************************/


/**************************************************************************/

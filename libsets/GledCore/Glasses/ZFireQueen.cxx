// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZFireQueen
//
//

#include "ZFireQueen.h"
#include "ZEunuch.h"
#include "ZFireQueen.c7"

ClassImp(ZFireQueen)

/**************************************************************************/

void ZFireQueen::_init()
{
  mEunuchs = 0;
}

/**************************************************************************/

void ZFireQueen::bootstrap()
{
  PARENT_GLASS::bootstrap();

  ZHashList* l = new ZHashList("Eunuchs");
  l->SetElementFID(ZEunuch::FID());
  CheckIn(l); SetEunuchs(l); l->SetMIRActive(false);

  ZNameMap* nm = new ZNameMap("Etc");
  CheckIn(nm); Add(nm);
}

/**************************************************************************/

ZGlass* ZFireQueen::DemangleID(ID_t id){
  // This should serve to properly demangle external references for comets.
  // FireQueens ignore dependencies.

  return mSaturn->DemangleID(id);
}

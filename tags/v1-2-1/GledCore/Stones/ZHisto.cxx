// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZHisto.h"
#include <Stones/ZHistoManager.h>
#include <Glasses/ZGlass.h>
#include <Ephra/Saturn.h>

ClassImp(ZHisto)

const ZGlass* ZHisto::GetGlass() const
{
  return dynamic_cast<const ZGlass*>(this);
}

ZHistoManager* ZHisto::GetZHistoManager()
{
  if(mManager) return mManager;
  return GetGlass()->GetSaturn()->GetZHistoManager();
}

ZHistoDir* ZHisto::GetHistoDir()
{
  return GetZHistoManager()->GetDir(this);
}

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZHistoDir.h"
#include <Glasses/ZGlass.h>
#include <Gled/GledNS.h>

ClassImp(ZHistoDir)

ZHistoDir::ZHistoDir(const ZHisto* n, const Text_t* name, const Text_t* title) :
  mHisto(n)
{
  mDir = new TDirectory(name, title);
}

ZHistoDir::~ZHistoDir() {}
/* this crashed root ...
  for(mName2HistoGroup_i i=mName2Group.begin(); i!=mName2Group.end(); i++)
    delete i->second;
*/

void ZHistoDir::AddGroup(const Text_t* name, const Text_t* title)
{
  if(mName2Group[name]) {
    ISerr(GForm("ZHistoDir::AddGroup Registering ZHistoGroup %s for %s *again* ... skip",
		name, mHisto->GetGlass()->GetName()));
    return;
  }
  GledNS::PushFD(); mDir->cd();
  mName2Group[name] = new ZHistoGroup(name, title);
  GledNS::PopFD();
}

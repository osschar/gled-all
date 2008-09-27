// $Header

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZHistoGroup.h"
#include <TH1.h>

ClassImp(ZHistoGroup)

ZHistoGroup::ZHistoGroup(const Text_t* name, const Text_t* title)
{
  mDir = new TDirectory(name, title);
}

void ZHistoGroup::AddHisto(TH1** p) { mHistos.push_back(p); }

void ZHistoGroup::ResetHistos()
{
  for(vppHisto_i i=mHistos.begin(); i!=mHistos.end(); i++) {
    (*(*i))->Reset();
  }
}

/**************************************************************************/
// Histo callbacks
/**************************************************************************/

void HistoDraw_cb(void* w, TObject* histo) {
  if(TH1* h = dynamic_cast<TH1*>(histo)) {
    h->Draw();
  }
}

void HistoReset_cb(void* w, TObject* histo) {
  if(TH1* h = dynamic_cast<TH1*>(histo)) {
    h->Reset();
  }
}

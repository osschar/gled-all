// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GLED_ZHistoGroup_H
#define GLED_ZHistoGroup_H

#include <Gled/GledTypes.h>
#include <TDirectory.h>
class TH1;
#include <vector>

#ifndef __CINT__
typedef vector<TH1**>			vppHisto_t;
typedef vector<TH1**>::iterator		vppHisto_i;
#endif

class ZHistoGroup {

protected:
  TDirectory*		mDir;		// X{g}

#ifndef __CINT__
  vppHisto_t		mHistos;
#endif

public:
  ZHistoGroup(const Text_t* name, const Text_t* title);
  virtual ~ZHistoGroup() {}

  void AddHisto(TH1** p);
  void ResetHistos();

  void cd() { mDir->cd(); }

#include "ZHistoGroup.h7"
  ClassDef(ZHistoGroup, 0);
}; // endclass ZHistoGroup

// Here I declare callbacks for Draw, Reset ... should be somewhere else
// Perhaps namespace HistoFoo ?? ... better
void HistoDraw_cb(void* w, TObject* histo);
void HistoReset_cb(void* w, TObject* histo);

#endif

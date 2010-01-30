// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GLED_ZHistoManager_H
#define GLED_ZHistoManager_H

#include <Gled/GledTypes.h>
#include <Stones/ZHistoDir.h>
#include <Stones/ZHisto.h>
#include <TFile.h>

#ifndef __CINT__
typedef map<ZHisto*, ZHistoDir*>		mHisto2ZHistoDir_t;
typedef map<ZHisto*, ZHistoDir*>::iterator	mHisto2ZHistoDir_i;
#endif

// ?? Should subclass SaturnService

class ZHistoManager : public TFile {
private:
#ifndef __CINT__
  mHisto2ZHistoDir_t	mHisto2HistoDir;
#endif

public:
  ZHistoManager(const char* fname, Option_t* option="",
		const char* ftitle="", Int_t compress = 1);
  virtual ~ZHistoManager();

  void RegisterGroup(ZHisto *n, const Text_t* group);

  ZHistoDir*   GetDir(ZHisto* n);
  ZHistoGroup* GetGroup(ZHisto* n, const Text_t* group);

#include "ZHistoManager.h7"
  ClassDef(ZHistoManager, 0);
}; // endclass ZHistoManager

#endif

// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GLED_ZHistoDir_H
#define GLED_ZHistoDir_H

#include <Gled/GledTypes.h>
#include <Stones/ZHisto.h>
#include <Stones/ZHistoGroup.h>
#include <TDirectory.h>

#ifndef __CINT__
typedef map<const Text_t*, ZHistoGroup*>		mName2HistoGroup_t;
typedef map<const Text_t*, ZHistoGroup*>::iterator	mName2HistoGroup_i;
#endif

class ZHistoDir {

protected:
  TDirectory*		mDir;		// X{g}

  const ZHisto*		mHisto;		// X{g}
#ifndef __CINT__
  mName2HistoGroup_t	mName2Group;
#endif

public:
  ZHistoDir(const ZHisto* n, const Text_t* name, const Text_t* title);
  virtual ~ZHistoDir();

  void AddGroup(const Text_t* name, const Text_t* title);
  ZHistoGroup* GetGroup(const Text_t* name) { return mName2Group[name]; }

  void cd() { mDir->cd(); }

#include "ZHistoDir.h7"
  ClassDef(ZHistoDir, 0)
}; // endclass ZHistoDir

#endif

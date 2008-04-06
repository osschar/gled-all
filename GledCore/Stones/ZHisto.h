// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GLED_ZHisto_H
#define GLED_ZHisto_H

#include <Gled/GledTypes.h>
#include <TString.h>
class ZGlass;
class ZHistoManager;
class ZHistoDir;

class ZHisto {

protected:
  ZHistoManager*	mManager;	//! X{gs}

public:
  ZHisto(ZHistoManager* m=0) : mManager(m) {}
  virtual ~ZHisto() {}

  virtual ZGlass* GetGlass();
  virtual ZHistoManager* GetZHistoManager();
  ZHistoDir* GetHistoDir();

  virtual void InitHistoGroups() = 0;
  virtual void ResetHistos() = 0;

#include "ZHisto.h7"
  ClassDef(ZHisto, 1)
}; // endclass ZHisto

#endif

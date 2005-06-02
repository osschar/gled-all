// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Gled/GledTypes.h>

#include <TSystem.h>
#include <TROOT.h>
#include <TInterpreter.h>

void libAlice_GLED_user_init()
{
  const Text_t* alishare = gSystem->Getenv("ALIGLEDSHARE");
  if(alishare) {
    gROOT->SetMacroPath(GForm("%s:%s/macros", gROOT->GetMacroPath(),
			      alishare));
    gInterpreter->AddIncludePath(GForm("%s/macros", alishare));
  }
}

void *Alice_GLED_user_init = (void*)libAlice_GLED_user_init;

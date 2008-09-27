// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TTreeTools
//
//

#include "TTreeTools.h"

/**************************************************************************/
/**************************************************************************/

#include <TTreeFormula.h>

ClassImp(TSelectorToEventList)

TSelectorToEventList::TSelectorToEventList(TEventList* evl, const Text_t* sel) :
  TSelectorDraw(), fEvList(evl)
{
  fInput.Add(new TNamed("varexp", ""));
  fInput.Add(new TNamed("selection", sel));
  SetInputList(&fInput);
}

Bool_t TSelectorToEventList::Process(Long64_t entry)
{
  if(ProcessCut(entry)) { ProcessFill(entry); return true; }
  return false;
}

Bool_t TSelectorToEventList::ProcessCut(Long64_t entry)
{
  return GetSelect()->EvalInstance(0) != 0;
}

/**************************************************************************/
/**************************************************************************/

#include <TTree.h>

ClassImp(TTreeQuery)

Int_t TTreeQuery::Select(TTree* t, const Text_t* selection)
{
  TSelectorToEventList sel(this, selection);
  t->Process(&sel, "goff");
  return GetN();
}

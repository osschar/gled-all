// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SGlUniform.h"

// SGlUniform

//______________________________________________________________________________
//
//

ClassImp(SGlUniform);

//==============================================================================

SGlUniform::SGlUniform() :
  ZRCStone(),
  fIsFloat  (false),
  fType     (-1),
  fVarSize  (-1),
  fArrSize  (-1),
  fLocation (-1)
{}

SGlUniform::SGlUniform(const Text_t* name, const Text_t* defs, Bool_t is_float,
		       Int_t type, Int_t var_size, Int_t arr_size, Int_t loc) :
  ZRCStone(),

  fName     (name),
  fDefaults (defs),
  fIsFloat  (is_float),
  fType     (type),
  fVarSize  (var_size),
  fArrSize  (arr_size),
  fLocation (loc)
{}

SGlUniform::~SGlUniform()
{}

//==============================================================================

void SGlUniform::Reset(const TString& name, const TString& defs, Bool_t is_float,
		       Int_t type, Int_t var_size, Int_t arr_size, Int_t loc)
{
  fName     = name;
  fDefaults = defs;
  fIsFloat  = is_float;
  fType     = type;
  fVarSize  = var_size;
  fArrSize  = arr_size;
  fLocation = loc;
}

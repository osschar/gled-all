// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_SGlUniform_H
#define Geom1_SGlUniform_H

#include <Stones/SRefCounted.h>
#include <TString.h>

class SGlUniform : public SRefCounted
{
protected:

public:
  TString fName;
  TString fDefaults;
  Bool_t  fIsFloat;
  Int_t   fType;
  Int_t   fVarSize;
  Int_t   fArrSize;
  Int_t   fLocation;

  union DataPtr_u
  {
    Int_t   *fInt;
    Float_t *fFloat;
  };

public:
  SGlUniform();
  SGlUniform(const Text_t* name, const Text_t* defs, Bool_t is_float,
	     Int_t type, Int_t var_size, Int_t arr_size, Int_t loc);
  virtual ~SGlUniform();

  void Reset(const TString& name, const TString& defs, Bool_t is_float,
	     Int_t type, Int_t var_size, Int_t arr_size, Int_t loc);

#include "SGlUniform.h7"
  ClassDef(SGlUniform, 1);
}; // endclass SGlUniform

#endif

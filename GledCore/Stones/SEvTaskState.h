// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SEvTaskState_H
#define GledCore_SEvTaskState_H

#include <Gled/GledTypes.h>
#include <TObject.h>

class SEvTaskState : public TObject {
  MAC_RNR_FRIENDS(SEvTaskState);

private:
  void _init();

protected:
  Char_t	mState;	// X{GS} Can be 'W'aiting, 'R'unning or 'F'inished
  Int_t		mNAll;	// X{GS}
  Int_t		mNOK;	// X{GS}
  Int_t		mNFail;	// X{GS}
  Int_t		mNProc;	// X{GS}

public:
  SEvTaskState() : TObject() { _init(); }

  void Reinit(Int_t n);

  void IncNAll (Int_t dn) { mNAll  += dn; }
  void IncNOK  (Int_t dn) { mNOK   += dn; }
  void IncNFail(Int_t dn) { mNFail += dn; }
  void IncNProc(Int_t dn) { mNProc += dn; }

  Int_t GetNLeft() const { return mNAll - mNOK - mNFail - mNProc; }
  Int_t GetNToDo() const { return mNAll - mNOK - mNFail; }
  Int_t GetNDone() const { return mNOK + mNFail; }

  SEvTaskState& operator+=(const SEvTaskState& s);

#include "SEvTaskState.h7"
  ClassDef(SEvTaskState, 1)
}; // endclass SEvTaskState

#endif

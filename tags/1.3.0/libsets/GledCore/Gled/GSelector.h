// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GSelector_H
#define Gled_GSelector_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>
#include <map>

// FileDescriptor 2 UserData
typedef map<Int_t, void*>		mFdUD_t;
typedef map<Int_t, void*>::iterator	mFdUD_i;

// Probably should provide methods for adding fds ...

class GSelector : public GMutex {
public:
  enum Error_e { SE_Null=0, SE_BadFD, SE_Interrupt,
		 SE_BadArg, SE_NoMem, SE_Unknown };

  Error_e	fError;

  mFdUD_t	fRead;
  mFdUD_t	fWrite;
  mFdUD_t	fExcept;
  mFdUD_t	fReadOut;
  mFdUD_t	fWriteOut;
  mFdUD_t	fExceptOut;
  Float_t	fTimeOut;

public:
  GSelector(Init_e e=fast);
  virtual ~GSelector() {}

  Int_t Select();

#include "GSelector.h7"
  ClassDef(GSelector,0);
}; // endclass GSelector

#endif

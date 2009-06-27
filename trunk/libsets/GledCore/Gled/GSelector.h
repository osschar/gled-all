// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GSelector_H
#define Gled_GSelector_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>
#include <map>

class    TSocket;

struct GFdSet : public map<void*,Int_t>
{
  void Add(void* ud, Int_t fd);
  void Add(TSocket* s);
  void Remove(void* ud);
};

typedef GFdSet::iterator GFdSet_i;

class GSelector : public GMutex
{
public:
  enum Error_e { SE_Null=0, SE_BadFD, SE_Interrupt,
		 SE_BadArg, SE_NoMem, SE_Unknown };

  Error_e	fError;

  GFdSet	fRead;
  GFdSet	fWrite;
  GFdSet	fExcept;
  GFdSet	fReadOut;
  GFdSet	fWriteOut;
  GFdSet	fExceptOut;
  Float_t	fTimeOut;

public:
  GSelector(Init_e e=fast);
  virtual ~GSelector();

  Int_t Select();

#include "GSelector.h7"
  ClassDef(GSelector,0);
}; // endclass GSelector

#endif

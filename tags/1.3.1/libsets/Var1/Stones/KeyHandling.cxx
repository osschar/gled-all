// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "KeyHandling.h"

using namespace KeyHandling;

KeyInfo::KeyInfo(const TString& tag, const TString& desc, AKeyCallback* foo, Int_t uid) :
  fKeyTag(tag), fKeyDesc(desc), fCallback(foo), fUserId(uid), fIndex(-1), fDownCount(0)
{}

KeyInfo::~KeyInfo()
{
  delete fCallback;
}

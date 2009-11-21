// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "KeyHandling.h"

using namespace KeyHandling;

KeyInfo::KeyInfo() :
  fCallback(0), fIndex(-1), fDownCount(0)
{}

KeyInfo::KeyInfo(const TString& tag, const TString& desc, AKeyCallback* foo) :
  fKeyTag(tag), fKeyDesc(desc), fCallback(foo), fIndex(-1), fDownCount(0)
{}

KeyInfo::~KeyInfo()
{}

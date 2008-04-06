// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Forest.h"
#include <Glasses/ZNode.h>
#include <Ephra/Saturn.h>

ClassImp(Forest)

// Plain constructor ... no mother creation!!
Forest::Forest() {}

Forest::Forest(Saturn *s) : mSaturn(s) {}

Forest::Forest(Text_t* name, Text_t* title) : TNamed(name,title) {}

Forest::Forest(Saturn* s, Text_t* name, Text_t* title) : TNamed(name,title), mSaturn(s) {}

void Forest::Init()
{
}

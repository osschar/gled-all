// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SMessage.h"

// SMessage

//______________________________________________________________________________
//
// Wrapper around TMessage allowing somewhat more optimized use.

ClassImp(SMessage);

//==============================================================================

SMessage::SMessage(UInt_t what, Int_t bufsize) :
  TMessage(what, bufsize)
{}

SMessage::SMessage(void* buf, Int_t bufsize) :
  TMessage(buf, bufsize)
{}

SMessage::~SMessage()
{}

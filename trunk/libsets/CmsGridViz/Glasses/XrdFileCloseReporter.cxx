// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFileCloseReporter.h"
#include "XrdFileCloseReporter.c7"

// XrdFileCloseReporter

//______________________________________________________________________________
//
//

ClassImp(XrdFileCloseReporter);

//==============================================================================

void XrdFileCloseReporter::_init()
{}

XrdFileCloseReporter::XrdFileCloseReporter(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

XrdFileCloseReporter::~XrdFileCloseReporter()
{}

//==============================================================================

void XrdFileCloseReporter::FileClosed(XrdFile* file)
{
  // put into queue and signal condition


}

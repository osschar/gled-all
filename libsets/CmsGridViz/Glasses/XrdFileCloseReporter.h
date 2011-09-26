// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdFileCloseReporter_H
#define CmsGridViz_XrdFileCloseReporter_H

#include <Glasses/ZGlass.h>
#include <Glasses/XrdMonSucker.h>

class XrdFileCloseReporter : public ZGlass,
			     public XrdMonSucker::FileCloseAbsorber
{
  MAC_RNR_FRIENDS(XrdFileCloseReporter);

private:
  void _init();

protected:
  TString       mHost; // X{GS}
  UShort_t      mPort; // X{GS}

public:
  XrdFileCloseReporter(const Text_t* n="XrdFileCloseReporter", const Text_t* t=0);
  virtual ~XrdFileCloseReporter();

  // Virtual from XrdMonSucker::FileCloseAbsorber
  virtual void FileClosed(XrdFile* file);

#include "XrdFileCloseReporter.h7"
  ClassDef(XrdFileCloseReporter, 1);
}; // endclass XrdFileCloseReporter

#endif

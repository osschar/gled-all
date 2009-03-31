// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_PMSEntry_H
#define GledCore_PMSEntry_H

#include <Stones/ZTrans.h>
class ZNode;

struct PMSEntry {
  PMSEntry*   fPrev;
  ZNode*      fNode;

  ZTrans     fLocal;
  ZTrans*    fToGCS;
  ZTrans*    fFromGCS;
  Bool_t      bTo, bFrom;

  PMSEntry() : fPrev(0), fNode(0),
	       fToGCS(0), fFromGCS(0), bTo(0), bFrom(0) {}

  ~PMSEntry()
  { delete fToGCS; delete fFromGCS; }

  ZTrans& ToGCS();
  ZTrans& FromGCS();
};

#endif

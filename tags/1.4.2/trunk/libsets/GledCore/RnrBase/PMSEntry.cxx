// $Id$
#include "PMSEntry.h"

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
//
// PMSEntry -- Position-Matrix-Stack Entry
//
// Used within RnrDriver to store matrices that are used
// and during lens-graph traversal.
// It also caches transformation matrices to/from GCS - they are
// created on request when needed and invalidated when changed
// from the ZNode_GL_Rnr.
//
// In principle non-ZNodes could also use this service.

ZTrans& PMSEntry::ToGCS()
{
  if(fToGCS == 0) fToGCS = new ZTrans;

  if(bTo == false) {
    if(fPrev != 0) {
      *fToGCS  = fPrev->ToGCS();
      *fToGCS *= fLocal;
    } else {
      *fToGCS  = fLocal;
    }
    bTo = true;
  }
  return *fToGCS;
}

ZTrans& PMSEntry::FromGCS()
{
  if(fFromGCS == 0) fFromGCS = new ZTrans;

  if(bFrom == false) {
    *fFromGCS = ToGCS();
    fFromGCS->Invert();
    bFrom = true;
  }
  return *fFromGCS;
}

// $Header$
#include "PMSEntry.h"

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// PMSEntry
//

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
    fFromGCS->InvertFast();
    bFrom = true;
  }
  return *fFromGCS;
} 

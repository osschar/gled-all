// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_RnrScheme_H
#define Gled_RnrScheme_H

#include "A_Rnr.h"
#include <Eye/OptoStructs.h>

/**************************************************************************/
/**************************************************************************/

struct RnrElement {
  OptoStructs::A_GlassView* fView;
  A_Rnr*		    fRnr;
  RnrSubDraw_foo	    fRnrFoo;

  RnrElement(OptoStructs::A_GlassView* v) :
    fView(v), fRnr(0), fRnrFoo(0) {}
  RnrElement(A_Rnr* r, RnrSubDraw_foo f)  :
    fView(0), fRnr(r), fRnrFoo(f) {}
  RnrElement(OptoStructs::A_GlassView* v, RnrSubDraw_foo f)  :
    fView(v), fRnr(0), fRnrFoo(f) {}
};

typedef list<RnrElement>		lRnrElement_t;
typedef list<RnrElement>::iterator	lRnrElement_i;

typedef vector<lRnrElement_t>		vlRnrElement_t;
typedef vector<lRnrElement_t>::iterator	vlRnrElement_i;

/**************************************************************************/
/**************************************************************************/

// For optimization of updates, the fScheme could be split
// into fSelfScheme, fListScheme, fLinksScheme.
// then different rays would invalidate only the pertinent sub-schemes.

class RnrScheme {
public:
  vlRnrElement_t fScheme;

  RnrScheme(UChar_t size) : fScheme((int)size) {}
}; // endclass RnrScheme

#endif

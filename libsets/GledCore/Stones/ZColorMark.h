// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZColorMark_H
#define GledCore_ZColorMark_H

#include <Stones/ZColor.h>

class ZColorMark : public ZColor {

private:

protected:
  Float_t	mMark;

public:
  ZColorMark() : ZColor(), mMark(0) {}
  ZColorMark(Float_t m, const ZColor& c) : ZColor(c), mMark(m) {}
  ZColorMark(Float_t m, Float_t r, Float_t g, Float_t b, Float_t a = 1) :
    ZColor(r,g,b,a), mMark(m) {}

  Float_t m() const { return mMark; }
  void m(Float_t m) { mMark = m; }

  Float_t mark() const { return mMark; }
  void mark(Float_t m) { mMark = m; }

#include "ZColorMark.h7"
  ClassDef(ZColorMark, 1);
}; // endclass ZColorMark

#endif

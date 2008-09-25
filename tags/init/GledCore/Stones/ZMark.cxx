// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZMark.h"

ClassImp(ZMark)

#include <iomanip>

ostream& operator<<(ostream& s, ZMark& m) {
  s.setf(ios::fixed, ios::floatfield); s.precision(3);
  s << "X=(";
  for(UCIndex_t i=0; i<5; i++) s << m.X(i) << ((i==4) ? ")" : ", ");
  s << "\nS=(";
  for(UCIndex_t i=0; i<5; i++) s << m.S(i) << ((i==4) ? ")" : ", ");
  return s;
}

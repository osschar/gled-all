// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Z3Mark.h"

ClassImp(Z3Mark)

#include <iomanip>

ostream& operator<<(ostream& s, Z3Mark& m) {
  s.setf(ios::fixed, ios::floatfield); s.precision(3);
  s << "R=(";
  for(UCIndex_t i=0; i<3; i++) s << m.R(i) << ((i==2) ? ")" : ", ");
  s << "\tS=(";
  for(UCIndex_t i=0; i<3; i++) s << m.S(i) << ((i==2) ? ")" : ", ");
  return s;
}
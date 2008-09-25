// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZVectorD.h"
ClassImp(ZVectorD)

#include <Stones/ZVector.h>

ZVectorD::ZVectorD(ZVector& x) : TVectorD(x.GetNrows()) {
  *this = x;
}

ZVectorD& ZVectorD::operator=(const ZVector& source)
{
  if(GetNrows() != source.GetNrows())
    ResizeTo(source.GetNrows());
  for(UCIndex_t i=0; i<GetNrows(); i++)
    fElements[i] = source(i);
  return *this;
}

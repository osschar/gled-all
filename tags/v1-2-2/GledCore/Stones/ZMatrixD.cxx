// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZMatrixD.h"
ClassImp(ZMatrixD)

#include <Stones/ZMatrix.h>

ZMatrixD::ZMatrixD(const ZMatrix& x) : TMatrixD(x.GetNrows(), x.GetNcols()) {
  *this = x;
}

ZMatrixD& ZMatrixD::operator=(const ZMatrix& source)
{
  if(GetNrows() != source.GetNrows() || GetNcols() != source.GetNcols())
    ResizeTo(source.GetNrows(), source.GetNcols());
  for(UInt_t i=0; i<GetNrows(); i++)
    for(UInt_t j=0; j<GetNcols(); j++)
      At(i,j) = source(i,j);
  return *this;
}

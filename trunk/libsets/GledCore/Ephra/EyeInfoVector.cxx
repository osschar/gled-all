// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "EyeInfoVector.h"

// EyeInfoVector

//______________________________________________________________________________
//
//

ClassImp(EyeInfoVector);

//==============================================================================

EyeInfoVector* EyeInfoVector::CloneAndAdd(EyeInfoVector* eiv, EyeInfo* ei)
{
  const Int_t N = eiv->size();
  EyeInfoVector *v = new EyeInfoVector(N + 1);
  for (Int_t i = 0; i < N; ++i)
  {
    (*v)[i] = (*eiv)[i];
  }
  (*v)[N] = ei;
  v->IncRefCnt();
  eiv->DecRefCnt();
  return v;
}

EyeInfoVector* EyeInfoVector::CloneAndRemove(EyeInfoVector* eiv, EyeInfo* ei)
{
  const Int_t N = eiv->size();
  EyeInfoVector *v = new EyeInfoVector(N - 1);
  for (Int_t i = 0, j = 0; i < N; ++i)
  {
    if ((*eiv)[i] != ei)
      (*v)[j++] = (*eiv)[i];
  }
  v->IncRefCnt();
  eiv->DecRefCnt();
  return v;
}

//==============================================================================

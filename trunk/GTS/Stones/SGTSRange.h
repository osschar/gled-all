// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_SGTSRange_H
#define GTS_SGTSRange_H

#include <TObject.h>

class SGTSRange : public TObject {

private:
  void _init();

protected:
  Double_t	mMin;   // X{GS}
  Double_t	mMax;   // X{GS}
  Double_t	mAvg;   // X{GS}
  Double_t	mSigma; // X{GS}

public:
  SGTSRange() : TObject()
  { _init(); }


#include "SGTSRange.h7"
  ClassDef(SGTSRange, 1)
}; // endclass SGTSRange

#endif

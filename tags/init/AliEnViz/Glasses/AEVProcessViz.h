// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVProcessViz_H
#define AliEnViz_AEVProcessViz_H

#include <Glasses/WSTube.h>
#include "AEVEventBatch.h"

class AEVProcessViz : public WSTube
{
  MAC_RNR_FRIENDS(AEVProcessViz);

private:
  void _init();

protected:
  ZLink<AEVEventBatch> mBatch; // X{GS} L{}

  Int_t   mEntsDone; // X{GS} 7 ValOut(-join=>1)
  Float_t mMegsDone; // X{GS} 7 ValOut()

public:
  AEVProcessViz(const Text_t* n="AEVProcessViz", const Text_t* t=0) :
    WSTube(n,t) { _init(); }


#include "AEVProcessViz.h7"
  ClassDef(AEVProcessViz, 1)
}; // endclass AEVProcessViz


#endif

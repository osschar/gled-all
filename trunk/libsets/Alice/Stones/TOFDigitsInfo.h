// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TOFDigitsInfo_H
#define Alice_TOFDigitsInfo_H

#include <Stones/ZRCStone.h>
#include <TClonesArray.h>

#include <TTree.h>
#include <AliTOFGeometry.h>


class TOFDigitsInfo : public ZRCStone {

 private:
  void _init();

 protected:
  TString                    mDataDir;  // X{G} 
  Int_t                      mEvent;    // X{G}

 public:
  TTree*                     mTree;
  AliTOFGeometry*            mGeom; 
  TClonesArray*              mDigits;

  TOFDigitsInfo(const Text_t* n="TOFDigitsInfo", const Text_t* t=0) :
    ZRCStone()
  { _init(); }

  void SetData(const Text_t* data_dir, Int_t event);


#include "TOFDigitsInfo.h7"
  ClassDef(TOFDigitsInfo, 1)
    }; // endclass TOFDigitsInfo

#endif

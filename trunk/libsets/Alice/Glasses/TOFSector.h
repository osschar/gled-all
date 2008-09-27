// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TOFSector_H
#define Alice_TOFSector_H

#include <Glasses/ZNode.h>
#include <Stones/TOFDigitsInfo.h>

class TOFSector : public ZNode {
  MAC_RNR_FRIENDS(TOFSector);

 private:
  void _init();

 protected:
  TOFDigitsInfo*    mInfo;
  Int_t             mSecID;     // X{gS}  7 Value(-range=>[0,17,1], -join=>1)

 public:
  TOFSector(const Text_t* n="TOFSector", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  TOFSector(Int_t sec, TOFDigitsInfo* info);

#include "TOFSector.h7"
  ClassDef(TOFSector, 1)
    }; // endclass TOFSector


#endif

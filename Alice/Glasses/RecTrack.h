// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_RecTrack_H
#define Alice_RecTrack_H

#include <Glasses/ZNode.h>
#include <Stones/ESDTrack.h>


class RecTrack : public ZNode {
  MAC_RNR_FRIENDS(RecTrack);

 private:
  void _init();

 protected:
  ESDTrack*                mESD;           // X{GS} 
  TString                  mV;             // X{GS}  7 TextOut()
  TString                  mP;             // X{GS}  7 TextOut()

 public:
  RecTrack(const Text_t* n="RecTrack", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  RecTrack(ESDTrack* esd, const Text_t* n="RecTrack", const Text_t* t=0);
  void Dump();                            // X{E} 7 MButt()

#include "RecTrack.h7"
  ClassDef(RecTrack, 1)
    }; // endclass RecTrack

GlassIODef(RecTrack);

#endif

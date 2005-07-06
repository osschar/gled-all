// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_KinkTrack_H
#define Alice_KinkTrack_H

#include <Glasses/TrackBase.h>
#include <Stones/Kink.h>
 
class KinkTrack : public TrackBase {
  MAC_RNR_FRIENDS(KinkTrack);

 private:
  void _init();

 protected:
  Kink*             mKink;           // X{GS} 
 public:
  KinkTrack(const Text_t* n="KinkTrack", const Text_t* t=0) :
    TrackBase(n,t) { _init(); }

  KinkTrack(Kink* kink, const Text_t* n="KinkTrack", const Text_t* t=0);
  virtual ~KinkTrack() { delete mKink; }

#include "KinkTrack.h7"
  ClassDef(KinkTrack, 1)
    }; // endclass KinkTrack

GlassIODef(KinkTrack);

#endif

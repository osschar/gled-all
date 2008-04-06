// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVFlatSSpace_H
#define AliEnViz_AEVFlatSSpace_H

#include <Glasses/Board.h>
#include <AliEnViz/AEVSSpace_ABase.h>

class AEVSite;

class AEVFlatSSpace : public Board, public AEVSSpace_ABase
{
  MAC_RNR_FRIENDS(AEVFlatSSpace);

private:
  void _init();

protected:
  // These should in principle be ValOuts!
  Float_t     mTheta0; // X{GS} 7 Value(-range=>[-90,90], -join=>1)
  Float_t     mDTheta; // X{GS} 7 Value(-range=>[0,180])
  Float_t     mPhi0;   // X{GS} 7 Value(-range=>[-180,180], -join=>1)
  Float_t     mDPhi;   // X{GS} 7 Value(-range=>[0,360])

public:
  AEVFlatSSpace(const Text_t* n="AEVFlatSSpace", const Text_t* t=0) :
    Board(n,t) { _init(); }

  virtual Bool_t ImportSite(AEVSite* site, Bool_t warn=false); // X{E} C{1}
  virtual void   ClearSiteVizes(); // X{E}

  void ImportAllSites();             // X{E} 7 MButt()

#include "AEVFlatSSpace.h7"
  ClassDef(AEVFlatSSpace, 1)
}; // endclass AEVFlatSSpace


#endif

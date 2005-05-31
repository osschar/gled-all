// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_MCTrack_H
#define Alice_MCTrack_H


#include <Glasses/TrackBase.h>
#include <Stones/ZColor.h>
#include <Stones/MCParticle.h>

#include <TParticle.h>
#include <AliHit.h>

class ZAliLoad;

class MCTrack : public TrackBase {
  MAC_RNR_FRIENDS(MCTrack);

 private:
  void _init();
 protected:
  MCParticle*		   mParticle; 	   // X{GS}  
  TString                  mVDecay;        // X{GS}  7 TextOut()
  Int_t                    mNDaughters;    // X{GS}  7 ValOut()

 public:
  MCTrack(const Text_t* n="MCTrack", const Text_t* t=0) :
    TrackBase(n,t) { _init(); }

  MCTrack(MCParticle* p, const Text_t* n="MCTrack", const Text_t* t=0);
  virtual ~MCTrack() { delete mParticle; }

  // particles
  void ImportDaughters(ZAliLoad* alil = 0);    //X{E} C{1} 7 MButt(-join=>1)
  void ImportDaughtersRec(ZAliLoad* alil = 0); //X{E} C{1} 7 MButt()
  void ImportHits(ZAliLoad* alil = 0);              // X{Ed} C{1} 7 MButt(-join=>1)
  void ImportHitsFromPrimary(ZAliLoad* alil = 0);   // X{Ed} C{1} 7 MButt()
  void Dump();				            // X{E} 7 MButt()
	
 
#include "MCTrack.h7"
  ClassDef(MCTrack, 1)
    }; // endclass MCTrack

GlassIODef(MCTrack);

#endif

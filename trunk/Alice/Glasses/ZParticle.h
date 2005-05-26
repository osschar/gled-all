// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ZParticle_H
#define Alice_ZParticle_H


#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/MCParticle.h>

#include <TParticle.h>
#include <AliHit.h>

class ZAliLoad;

class ZParticle : public ZNode {
  MAC_RNR_FRIENDS(ZParticle);

 private:
  void _init();
 protected:
  MCParticle*		   mParticle; 	   // X{GS}  
  TString                  mV;             // X{GS}  7 TextOut()
  TString                  mP;             // X{GS}  7 TextOut()
  TString                  mVDecay;        // X{GS}  7 TextOut()
  Int_t                    mNDaughters;    // X{GS}  7 ValOut()

 public:
  ZParticle(const Text_t* n="ZParticle", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  ZParticle(MCParticle* p, const Text_t* n="ZParticle", const Text_t* t=0);
  virtual ~ZParticle() { delete mParticle; }

  // particles
  void ImportDaughters(ZAliLoad* alil = 0);    //X{E} C{1} 7 MButt(-join=>1)
  void ImportDaughtersRec(ZAliLoad* alil = 0); //X{E} C{1} 7 MButt()
  void ImportHits(ZAliLoad* alil = 0);              // X{Ed} C{1} 7 MButt(-join=>1)
  void ImportHitsFromPrimary(ZAliLoad* alil = 0);   // X{Ed} C{1} 7 MButt()
  void Dump();				            // X{E} 7 MButt()
	
 
#include "ZParticle.h7"
  ClassDef(ZParticle, 1)
    }; // endclass ZParticle

GlassIODef(ZParticle);

#endif

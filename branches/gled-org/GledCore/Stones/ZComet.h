// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZComet_H
#define Gled_ZComet_H

#include <Gled/GledTypes.h>
#include <TNamed.h>

#include <Glasses/ZGlass.h>

class Saturn;
class ZKing; class ZQueen;
class ZList;
class TBuffer;

class ZComet : public TNamed {
  // **** Custom Streamer ****
public:
  enum CometType_e { CT_CometBag=0, CT_Queen, CT_King };

private:
  bool		bFail;
  bool		bGraphRebuilt;

  void		_init();

public:
#ifndef __CINT__
  mID2pZGlass_t mIDMap;
#endif

  CometType_e	mType;

  Saturn*	mSaturn;
  bool		bUseSaturn;
  bool		bWarnOn;	// X{GS}
  bool		bVerbose;	// X{GS}

  set<LID_t>	mLibSets;	// X{R}
  lpZGlass_t	mTopLevels;
  ZQueen*	mQueen;		// X{G}
  ZKing*	mKing;		// X{G}
  lpZGlass_t	mOrphans;

  sStr_t	mIgnoredLinks;

public:
  ZComet(const Text_t* n="Comet", const Text_t* t=0) : TNamed(n,t) { _init(); }
  virtual ~ZComet() {}

  Int_t AddTopLevel(ZGlass* g, Bool_t do_links, Bool_t do_lists, Int_t depth);
  Int_t AddGlass(ZGlass* g, Bool_t do_links=false, Bool_t do_lists=false,
		 Int_t depth=0);

  ZGlass* FindID(ID_t id);

  //Int_t SaturnShot();
  //Int_t BeamDown(const Text_t* keyname);

  void  UseSaturn(Saturn* sat);
  Int_t	RebuildGraph();

  void StreamHeader(TBuffer& b);
  void StreamContents(TBuffer& b);

#include "ZComet.h7"
  ClassDef(ZComet, 1)
}; // endclass ZComet

#endif

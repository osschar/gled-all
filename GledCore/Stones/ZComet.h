// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZComet_H
#define GledCore_ZComet_H

#include <Gled/GledTypes.h>
#include <TNamed.h>

#include <Glasses/ZGlass.h>

class Saturn;
class ZKing; class ZQueen;
class ZList;
class TBuffer;

class ZComet : public TNamed, public An_ID_Demangler
{
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

  CometType_e	mType;		// X{g}

  An_ID_Demangler* mExtDemangler; // X{gS}

  bool		bWarnOn;	// X{gS}
  bool		bVerbose;	// X{gS}

  set<LID_t>	mLibSets;	// X{R}
  lpZGlass_t	mTopLevels;
  ZQueen*	mQueen;		// X{g}
  ZKing*	mKing;		// X{g}
  lpZGlass_t	mOrphans;

  sStr_t	mIgnoredLinks;

public:
  ZComet(const Text_t* n="Comet", const Text_t* t=0) : TNamed(n,t) { _init(); }
  virtual ~ZComet() {}

  Int_t AddTopLevel(ZGlass* g, Bool_t do_links, Bool_t do_lists, Int_t depth);
  Int_t AddGlass(ZGlass* g, Bool_t do_links=false, Bool_t do_lists=false,
		 Int_t depth=0);

  // An_ID_Demangler; also affected by mExtDemangler
  virtual ZGlass* DemangleID(ID_t id);

  // void ReplaceIDEntry(ID_t id, ZGlass* lens);

  void  AssignQueen(ZQueen* queen);
  Int_t	RebuildGraph();

  void StreamHeader(TBuffer& b);
  void StreamContents(TBuffer& b);

#include "ZComet.h7"
  ClassDef(ZComet, 1)
}; // endclass ZComet

#endif

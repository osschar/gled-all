// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVAlienUI_H
#define AliEnViz_AEVAlienUI_H

#include <Glasses/ZNameMap.h>
#include <AliEnViz/AEV_NS.h>

class AEVAlienUI : public ZNameMap {
  MAC_RNR_FRIENDS(AEVAlienUI);

private:
  void _init();
  GMutex           hAliEnLock;

protected:
  ZLink<ZNameMap>  mSites;     // X{GS} L{}
  ZLink<ZNameMap>  mCompEls;   // X{GS} L{}

  ZLink<ZGlass>	   mPROOF;     // X{GS} L{}

  Bool_t	   bConnected; // X{GS} 7 BoolOut(-join=>1)
  Bool_t	   bLocked;    // X{GS} 7 BoolOut()

  AEV_NS::BiDiPipe mBDP;  //!

  void alien_lock()   { hAliEnLock.Lock();   bLocked = true;  }
  void alien_unlock() { hAliEnLock.Unlock(); bLocked = false; }


public:
  AEVAlienUI(const Text_t* n="AlienUI", const Text_t* t=0) :
    ZNameMap(n,t), hAliEnLock(GMutex::recursive)
  { _init(); }

  ~AEVAlienUI();

  virtual void AdEnlightenment();

  void Connect();    // X{E} 7 MButt(-join=>1)
  void Disconnect(); // X{E} 7 MButt()

  void ImportSites(const Text_t* partition=0); // X{Ed} 7 MButt()
  void SaveSites(const Text_t* file="alice-sites.root"); // X{E} 7 MButt(-join=>1)
  void LoadSites(const Text_t* file="alice-sites.root"); // X{E} 7 MButt()

  // w/ glite would have:
  // TGridResult* FindEx(...);
  void FindExCountFilesPerSite(map<string,int>& cmap, const Text_t* path, const Text_t* wcard);

#include "AEVAlienUI.h7"
  ClassDef(AEVAlienUI, 1)
}; // endclass AEVAlienUI


#endif

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_DigitsGUI_H
#define Alice_DigitsGUI_H

#include <Glasses/ZNode.h>
#include <Glasses/ZAliLoad.h>
#include <Glasses/ITSDigRnrMod.h>


class DigitsGUI : public ZNode {
  MAC_RNR_FRIENDS(DigitsGUI);

 private:
  void _init();
  ZNode* show_this_lens(const string& url, ZNode* holder);
  
  Bool_t             bSPDImport;
  Bool_t             bSSDImport;
  Bool_t             bSDDImport;

  Bool_t             bTPCImport;
 protected:
  // pointers to event display interface
  ZAliLoad*          mZAliLoad;   // X{GS}
  ITSDigRnrMod*      mITSRM;      // X{GS}
  ZNode*             mDigits;

  // menues
  ZNode*             mMainMenu;

  ZNode*             mITSMenu;
  ZNode*             mITSSubMenu;
  ZNode*             mITSDigits;
  ZNode*             mITSSubDigits;
 
  ZNode*             mTPCMenu;
  ZNode*             mTPCDigits;
 public:
  DigitsGUI(const Text_t* n="DigitsGUI", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  // create manues and hide it
  void Init();

  // callbacks
  void ITSMenu();  // X{E}
  void MainMenu(); // X{E}
  void SPDMenu();  // X{E}
  void SDDMenu();  // X{E}
  void SSDMenu();  // X{E}
  void InnerLayer();  // X{E}
  void OuterLayer();  // X{E}

  void TPCMenu();  // X{E}
#include "DigitsGUI.h7"
  ClassDef(DigitsGUI, 1)
    }; // endclass DigitsGUI

GlassIODef(DigitsGUI);

#endif

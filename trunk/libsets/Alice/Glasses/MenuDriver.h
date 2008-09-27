// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_MenuDriver_H
#define Alice_MenuDriver_H

#include <Glasses/ZList.h>
#include <Glasses/Scene.h>
#include <Glasses/WGlDirectory.h>
#include <Glasses/ScreenText.h>

#include <Glasses/Menu.h>
#include <Glasses/ZAliLoad.h>
#include <Glasses/ITSDigRnrMod.h>
#include <Glasses/TPCSegRnrMod.h>
#include <Glasses/TPCPadRow.h>


class MenuDriver : public WGlDirectory
{
  MAC_RNR_FRIENDS(MenuDriver);

 private:
  void _init();

 protected:
  Scene*               mOverlay;    //! X{GS}
  ScreenText*          mScreenText; //! X{GS}

  Menu*                mCurrent;    //! X{GS}
  Menu*                mPrevious;   //! X{GS}
  
  ZLink<ZNode>         mCanvas3D;   //  X{GS} L{}
  ZLink<ZNode>         mCanvas2D;   //  X{GS} L{}
  ZLink<ZNode>         mMDir;       //  X{GS} L{}

  
  // pointers to event display interface
  ZAliLoad*            mZAliLoad;   //  X{GS}

  ZLink<TPCSegRnrMod>  mTPCPlateRM; // X{GS} L{}
  ZLink<TPCSegRnrMod>  mTPCSegRM;   // X{GS} L{}

  ZLink<ITSDigRnrMod>  mITSRM;      // X{GS} L{}

 public:
  MenuDriver(const Text_t* n="MenuDriver", const Text_t* t=0) :
    WGlDirectory(n,t) { _init(); }

  void Init();
  void SwitchSelected(Menu* menu = 0);        // X{Ed} C{1}
  void SetParentToSubMenues(Menu* menu = 0);  // X{E} C{1}
  void SwitchViewMenu(Menu* menu = 0);        // X{E} C{1}

  //callbacks for menues
  void ITSMenu(ZNode* dh = 0, ZNode* lh = 0);  // X{E} C{2}
  void SPDMenu(ZNode* dh = 0, ZNode* lh = 0);  // X{E} C{2}
  void SDDMenu(ZNode* dh = 0, ZNode* lh = 0);  // X{E} C{2}
  void SSDMenu(ZNode* dh = 0, ZNode* lh = 0);  // X{E} C{2}
  void SelectITSLayer(ZNode* dh = 0, ZNode* lh = 0);  // X{E} C{2}
  void ITSModuleMenu(ZNode* dh = 0,ZNode* lh = 0);// X{E} C{2}

  void TPCMenu(ZNode* dh = 0, ZNode* lh = 0);   // X{E} C{2}
  void TPCSegMenu(ZNode* dh = 0,ZNode* lh = 0); // X{E} C{2}

  // callback for links
  void TPCSegPickMenu(ZGlass* lens = 0);        // X{E} C{1} 
  void ITSModPickMenu(ZGlass* lens = 0);        // X{E} C{1}
  void TPCSegFocus(ZGlass* lens =0);            // X{E} C{1} 

  void Exit();  // X{E}
  void Dump();

#include "MenuDriver.h7"
  ClassDef(MenuDriver, 1)
}; // endclass MenuDriver


#endif

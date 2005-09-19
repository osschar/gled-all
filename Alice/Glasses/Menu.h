// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_Menu_H
#define Alice_Menu_H

#include <Glasses/ZNode.h>
#include <Glasses/ZQueen.h>

class Menu : public ZList {
  MAC_RNR_FRIENDS(Menu);

 private:
  void _init();

 protected:
  Menu*    mParent;          //  X{GS} L{}
  ZNode*   mCanvas3D;         //  X{GS} L{}
  ZNode*   mCanvas2D;         //  X{GS} L{}
  ZList*   mSettings;         //  X{GS} L{}

  // mir memebers
  ZGlass*  mCreatorLens;     //  X{GS} L{}
  TString  mCreatorMethod;   //  X{GS} 
  ZGlass*  mOnEnterLens;     //  X{GS} L{}
  TString  mOnEnterMethod;   //  X{GS} 
  ZGlass*  mOnExitLens;     //  X{GS} L{}
  TString  mOnExitMethod;   //  X{GS} 

  Bool_t   mDigImport;       //  X{GS}
  Bool_t   mShowIn3D;        //  X{GS}
 public:
  Menu(const Text_t* n="Menu", const Text_t* t=0) :
    ZList(n,t) { 
      _init(); Init();  
    }
  void Init();
  void Dump();
  // virtual void     Init();
  virtual void     OnEnter();

#include "Menu.h7"
  ClassDef(Menu, 1)
    }; // endclass Menu

GlassIODef(Menu);

#endif

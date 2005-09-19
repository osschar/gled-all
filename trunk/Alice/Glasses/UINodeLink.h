// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_UINodeLink_H
#define Alice_UINodeLink_H

#include <Glasses/ZNodeLink.h>
#include <Gled/GledNS.h>

class DigitsGUI;

class UINodeLink : public ZNodeLink {
  MAC_RNR_FRIENDS(UINodeLink);

 private:
  void _init();

 protected:
  Bool_t              mActive;          //  X{GS}
  Bool_t              mHasFocus;        //  X{GS}

  ZGlass*             mCbackAlpha;      //  X{GS} L{}
  ZGlass*             mCbackBeta;       //  X{GS} L{}
  TString             mCbackMethodName; //  X{GS} Ray{CbackReset} 7 Textor()

  ZGlass*             mFocusAlpha;      //  X{GS} L{}
  ZGlass*             mFocusBeta;       //  X{GS} L{}
  TString             mFocusMethodName; //  X{GS} Ray{CbackReset} 7 Textor()

  GledNS::MethodInfo* mCbackMethodInfo; //! 
  GledNS::MethodInfo* mFocusMethodInfo; //! 
  

 public:  
  UINodeLink(const Text_t* n="UINodeLink", const Text_t* t=0) :
    ZNodeLink(n,t) { _init(); }

  GledNS::MethodInfo* GetCbackMethodInfo();
  GledNS::MethodInfo* GetFocusMethodInfo();
  void EmitCbackResetRay() { mCbackMethodInfo = 0; }
  //  void TPCSegMenu();    // X{E}

#include "UINodeLink.h7"
  ClassDef(UINodeLink, 1)
    }; // endclass UINodeLink

GlassIODef(UINodeLink);

#endif

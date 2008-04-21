// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlButton_H
#define GledCore_WGlButton_H

#include <Glasses/WGlWidget.h>
#include <Gled/GledNS.h>

class WGlButton : public WGlWidget
{
  MAC_RNR_FRIENDS(WGlButton);

private:
  void _init();

protected:
  ZLink<ZGlass>       mCbackBeta;       //  X{GS} L{}
  TString             mCbackMethodName; //  X{GS}     Ray{CbackReset} 7 Textor()
  GledNS::MethodInfo* mCbackMethodInfo; //!

public:
  WGlButton(const Text_t* n="WGlButton", const Text_t* t=0) :
    WGlWidget(n,t) { _init(); }

  virtual void EmitCbackResetRay() { mCbackMethodInfo = 0; }

  GledNS::MethodInfo* GetCbackMethodInfo();

  void MenuEnter(); // X{E}
  void MenuExit();  // X{E}

  void ExitGled();  // X{Ed}

#include "WGlButton.h7"
  ClassDef(WGlButton, 1);
}; // endclass WGlButton


#endif

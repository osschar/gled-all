// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ShellInfo_H
#define GledCore_ShellInfo_H

#include <Glasses/ZNameMap.h>
#include <Glasses/ZHashList.h>
#include <Glasses/SubShellInfo.h>
#include <Glasses/ZMirEmittingEntity.h>

#include <Net/Ray.h>

class ShellInfo : public ZNameMap {
  MAC_RNR_FRIENDS(ShellInfo);

public:
  enum PrivRayQN_e  { PRQN_offset = RayNS::RQN_user_0,
		      PRQN_set_def_subshell,
		      PRQN_add_subshell,
		      PRQN_remove_subshell,
		      PRQN_resize_window
  };

private:
  void _init();

protected:
  ZHashList*	mSubShells;	// X{gE} L{}
  SubShellInfo* mDefSubShell;   // X{gE} L{} 

  ZGlass*	mBeta;		// X{gS} L{}
  ZGlass*	mGamma;		// X{gS} L{}

  ZMirEmittingEntity* mMessageRecipient; // // X{gS} L{}

  Int_t		mDefW;		// X{GS} 7 Value(-range=>[32, 256, 1])
  Int_t		mDefSShellH;	// X{GS} 7 Value(-range=>[10, 128, 1])
  UChar_t	mMsgOutH;	// X{GS} Ray{Resize} 7 Value(-range=>[0,32,1])

  Bool_t	bFancyClassView;       // X{GS} 7 Bool()
  Bool_t	bCollZGlass;           // X{GS} 7 Bool(-join=>1)
  Bool_t	bCollZList;            // X{GS} 7 Bool()
  Bool_t	bShowLinksInClassView; // X{GS} 7 Bool()

public:
  ShellInfo(const Text_t* n="ShellInfo", const Text_t* t=0) :
    ZNameMap(n,t) { _init(); }

  virtual void AdEnlightenment();

  virtual SubShellInfo* MakeDefSubShell();

  virtual void AddSubShell(SubShellInfo* sub_shell);    // X{E} C{1} 7 MCWButt(-join=>1)
  virtual void RemoveSubShell(SubShellInfo* sub_shell); // X{E} C{1} 7 MCWButt()

  void SetDefSubShell(SubShellInfo* ss);
  void SetSubShells(ZHashList* ss);

  void EmitResizeRay();

#include "ShellInfo.h7"
  ClassDef(ShellInfo, 1) // Glass representation of GUI shell 'FTW_Shell'.
}; // endclass ShellInfo

GlassIODef(ShellInfo);

#endif

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ShellInfo_H
#define GledCore_ShellInfo_H

#include <Glasses/ZNameMap.h>
#include <Glasses/ZHashList.h>
#include <Glasses/SubShellInfo.h>
#include <Glasses/ZMirEmittingEntity.h>

#include <Eye/Ray.h>

class MetaViewInfo;

class ShellInfo : public ZNameMap
{
  MAC_RNR_FRIENDS(ShellInfo);

public:
  enum PrivRayQN_e  { PRQN_offset = RayNS::RQN_user_0,
		      PRQN_set_def_subshell,
		      PRQN_add_subshell,
		      PRQN_remove_subshell,
		      PRQN_spawn_classview,
		      PRQN_spawn_metagui,
		      PRQN_resize_window
  };

private:
  void _init();

protected:
  ZLink<ZHashList>    mSubShells;   // X{gE} L{}
  ZLink<SubShellInfo> mDefSubShell; // X{gE} L{}

  ZLink<ZGlass>	mBeta;		// X{gS} L{}
  ZLink<ZGlass>	mGamma;		// X{gS} L{}

  ZLink<ZMirEmittingEntity> mMessageRecipient; // // X{gS} L{}

  Int_t		mDefW;		// X{GS} 7 Value(-range=>[32, 256, 1])
  Int_t		mDefSShellH;	// X{GS} 7 Value(-range=>[10, 128, 1])
  Bool_t	bDefSourceVis;	// X{GS} 7 Bool(-join=>1)
  Bool_t	bDefSinkVis;	// X{GS} 7 Bool()

  UChar_t	mMsgOutH;	// X{GS} Ray{Resize} 7 Value(-range=>[0,32,1])

  Bool_t	bFancyClassView;       // X{GS} 7 Bool()
  Bool_t	bCollZGlass;           // X{GS} 7 Bool(-join=>1)
  Bool_t	bCollAList;            // X{GS} 7 Bool()
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

  void SpawnClassView(ZGlass* source); // X{E} C{1} 7 MCWButt()
  void SpawnMetaGui(ZGlass* source, MetaViewInfo* gui_template); // X{E} C{2} 7 MCWButt()

  void EmitResizeRay();

#include "ShellInfo.h7"
  ClassDef(ShellInfo, 1); // Glass representation of GUI shell 'FTW_Shell'.
}; // endclass ShellInfo


#endif

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GuiPupilInfo_H
#define GledCore_GuiPupilInfo_H

#include <Glasses/SubShellInfo.h>
#include <Glasses/PupilInfo.h>

class GuiPupilInfo : public SubShellInfo {
  MAC_RNR_FRIENDS(GuiPupilInfo);

private:
  void _init();

protected:
  ZLink<PupilInfo>	mPupil;   // X{gS} L{}
  ZLink<ZList>	mCameras; // X{gS} L{}

public:
  GuiPupilInfo(const Text_t* n="GuiPupilInfo", const Text_t* t=0) :
    SubShellInfo(n,t) { _init(); }

  void AssertDefaultPupil(); // X{E} 7 MButt()

#include "GuiPupilInfo.h7"
  ClassDef(GuiPupilInfo, 1);
}; // endclass GuiPupilInfo


#endif

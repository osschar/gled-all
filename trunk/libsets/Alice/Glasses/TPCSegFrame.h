// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TPCSegFrame_H
#define Alice_TPCSegFrame_H

#include <Glasses/UINodeLink.h>
#include <Stones/ZColor.h>
#include <AliTPCParam.h>
#include <Glasses/TPCPadRow.h>

class TPCSegFrame : public UINodeLink {
  MAC_RNR_FRIENDS(TPCSegFrame);

private:
  void _init();

protected:
  AliTPCParam*     mTPCPar;      // X{GS}
  TPCPadRow*       mPadRow;      // X{GS}
  ZColor           mRowCol;      // X{PGST} 7 ColorButt(-join=>1)
  ZColor           mFocusCol;    // X{PGST} 7 ColorButt(-join=>1)
public:
  TPCSegFrame(const Text_t* n="TPCSegFrame", const Text_t* t=0) :
    UINodeLink(n,t) { _init(); }

#include "TPCSegFrame.h7"
  ClassDef(TPCSegFrame, 1)
}; // endclass TPCSegFrame


#endif

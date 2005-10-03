// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZEunuch_H
#define GledCore_ZEunuch_H

#include <Glasses/ZHashList.h>
#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>

class ZEunuch : public ZHashList {
  MAC_RNR_FRIENDS(ZEunuch);

public:
  enum RequestType_e   { RT_Undef, RT_Pull, RT_Push };
  enum PushStrategy_e  { PS_Undef, PS_All, PS_Saturn, PS_SaturnList, 
			 PS_FollowSecQueen };
private:
  void _init();

protected:
  ZLink<ZQueen>	mPrimQueen;	// X{gS} L{}
  ZLink<ZQueen>	mSecQueen;	// X{gS} L{}

  ZLink<SaturnInfo>	mToSaturn;	// X{gS} L{}

  // Top level request type
  RequestType_e	 mRequest;	// X{gS} 7 PhonyEnum()

  // Push subdefs. Also mToSaturn, list contents and mSecQueen
  PushStrategy_e mPushStrategy;	// X{gS} 7 PhonyEnum()
  

public:
  ZEunuch(const Text_t* n="ZEunuch", const Text_t* t=0) : ZHashList(n,t)
  { _init(); }


#include "ZEunuch.h7"
  ClassDef(ZEunuch, 1)
}; // endclass ZEunuch


#endif

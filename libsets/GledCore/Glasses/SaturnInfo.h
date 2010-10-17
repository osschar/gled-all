// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SaturnInfo_H
#define GledCore_SaturnInfo_H

#include <Glasses/ZMirEmittingEntity.h>
class ZHashList;

#include <Gled/GCondition.h>

class ZQueen;
class EyeInfo;
class Saturn;
class TSocket;

class SaturnInfo : public ZMirEmittingEntity
{
  MAC_RNR_FRIENDS(SaturnInfo);

  friend class ZKing; friend class ZSunQueen; friend class ZQueen;
  friend class Saturn;

private:

  TSocket*	hSocket;	//! used locally in Saturn for routing.
  SaturnInfo*	hRoute;		//! likewise

  typedef set<ZQueen*>			spZQueen_t;
  typedef set<ZQueen*>::iterator	spZQueen_i;

  spZQueen_t	hQueens;	//! Queens mirrored to Saturn represented
				//  by this SaturnInfo (only used for moons of
				//  local Saturn)

protected:

  // Saturn config

  TString	mHostName;	// X{GSR} 7 TextOut()
  Int_t		mServerPort;	// X{GS}  7 ValOut(-range=>[0,65535,1], -join=>1)
  Int_t         mServPortScan;  // X{GS}  7 Value(-range=>[0,256,1])

  TString	mMasterName;	// X{GSR} 7 TextOut()
  Int_t		mMasterPort;	// X{GS}  7 ValOut(-range=>[0,65535,1,1])

  ID_t		mSunSpaceSize;	// X{GS}  7 ValOut(-range=>[0,MAX_ID,1,0], -width=>10)
  ID_t		mKingID;	// X{G}   7 ValOut(-range=>[0,MAX_ID,1,0], -width=>10)
  ID_t		mFireKingID;	// X{G}   7 ValOut(-range=>[0,MAX_ID,1,0], -width=>10)

  // Only relevant for Sun Absolute; set for all SaturnInfos on mee incarnation.
  Bool_t	bUseAuth;	// X{GS}  7 BoolOut()

  // Node info

  TString       mOS;            // X{GS} 7 TextOut()
  TString	mCPU_Model;	// X{GS} 7 TextOut()
  TString	mCPU_Type;	// X{GS} 7 TextOut()
  UShort_t	mCPU_Freq;	// X{GS} 7 ValOut(-width=>6, -join=>1)
  UShort_t	mCPU_Num;	// X{GS} 7 ValOut(-width=>6)

  UShort_t	mMemory;	// X{GS} 7 ValOut(-width=>6, -join=>1)
  UShort_t	mSwap;		// X{GS} 7 ValOut(-width=>6)

  UShort_t	mMFree;		// X{GS} 7 ValOut(-width=>6, -join=>1)
  UShort_t	mSFree;		// X{GS} 7 ValOut(-width=>6)

  Float_t	mLAvg1;		// X{GS} 7 ValOut(-width=>5, -join=>1)
  Float_t	mLAvg5;		// X{GS} 7 ValOut(-width=>5, -join=>1)
  Float_t	mLAvg15;	// X{GS} 7 ValOut(-width=>5)

  Float_t	mCU_Total;	// X{GS} 7 ValOut(-width=>6, -join=>1)
  Float_t	mCU_User;	// X{GS} 7 ValOut(-width=>6)
  Float_t	mCU_Nice;	// X{GS} 7 ValOut(-width=>6, -join=>1)
  Float_t	mCU_Sys;	// X{GS} 7 ValOut(-width=>6)
  Float_t	mCU_Idle;	// X{GS} 7 ValOut(-width=>6)

  // Cluster structure

  ZLink<SaturnInfo>	mMaster;	// X{gS} L{}
  ZLink<ZHashList>	mMoons;		// X{gS} L{}
  ZLink<ZHashList>	mEyes;		// X{gS} L{}

  void create_lists();

public:

  SaturnInfo(const Text_t* n="SaturnInfo", const Text_t* t=0);
  virtual ~SaturnInfo();

  virtual SaturnInfo* HostingSaturn() { return this; }
  // Virtuals exported from MEE
  virtual void Message(const Text_t* s);
  virtual void Warning(const Text_t* s);
  virtual void Error(const Text_t* s);

  void AddMoon(SaturnInfo* moon); // X{E} C{1}
  void AddEye(EyeInfo* eye);	  // X{E} C{1}

  void ReceiveBeamResult(UInt_t req_handle); // X{E} T{MEE::Self}

  void TellAverages(Int_t sample_time=500); // X{Ed}

#include "SaturnInfo.h7"
  ClassDef(SaturnInfo, 1);
}; // endclass SaturnInfo

#endif

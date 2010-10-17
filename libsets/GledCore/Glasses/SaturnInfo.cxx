// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SaturnInfo.h"
#include <Glasses/ZHashList.h>
#include <Glasses/EyeInfo.h>
#include "SaturnInfo.c7"

#include <Glasses/ZQueen.h>
#include <Glasses/EyeInfo.h>
#include <Stones/ZMIR.h>

#include <TSocket.h>
#include <TSystem.h>

//______________________________________________________________________
//
// SaturnInfo glass represents a Saturn within a Gled cluster.
//
// It is mostly a data-holder, functionality mostly in Saturn, Kings
// and Queens.

ClassImp(SaturnInfo);

SaturnInfo::SaturnInfo(const Text_t* n, const Text_t* t) :
  ZMirEmittingEntity(n,t)
{
  hSocket = 0; hRoute = 0;

  mHostName   = ""; mServerPort = GLED_DEF_PORT; mServPortScan = 0;
  mMasterName = ""; mMasterPort = GLED_DEF_PORT;

  mSunSpaceSize = mKingID = mFireKingID = 0;
  bUseAuth = false;

  mOS = mCPU_Model = mCPU_Type = "<unknown>";
  mCPU_Freq = mCPU_Num = 0;
  mMemory = mSwap = mMFree = mSFree = 0;
  mLAvg1 = mLAvg5 = mLAvg15 = 0;
  mCU_Total = mCU_User = mCU_Nice = mCU_Sys = mCU_Idle = 0;

  mMaster = 0; mMoons = 0; mEyes = 0;
}

SaturnInfo::~SaturnInfo()
{}

/**************************************************************************/

// !!! Caller id missing

void SaturnInfo::Message(const Text_t* s)
{
  ISmess(TString("Received message: ") + s);
}

void SaturnInfo::Warning(const Text_t* s)
{
  ISwarn(TString("Received warning: ") + s);
}

void SaturnInfo::Error(const Text_t* s)
{
  ISerr(TString("Received error: ") + s);
}

/**************************************************************************/

void SaturnInfo::create_lists()
{
  ZHashList* l;
  l = new ZHashList(GForm("Moons of %s", mName.Data()));
  mQueen->CheckIn(l); SetMoons(l);
  l = new ZHashList(GForm("Eyes of %s", mName.Data()));
  mQueen->CheckIn(l); SetEyes(l);
}

/**************************************************************************/
/**************************************************************************/



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/*
void SaturnInfo::handle_new_connection()
{
  // Called from local Saturn to manage login procedure for a new connection.
  // Beams SunAbsolute ... then waits for its responses.

  // create connection object

  // create beam ... send to SA

  // wait for response

  // if ok ... pass connection key, host, port to the new connectee

  // wait for response

  // if ok ... pass data to the new connectee
}

void SaturnInfo::new_connection_request(SaturnInfo* host)
{
  // Called by an SaturnInfo in an attempt to initiate new connection.
  // Must be sent to SunAbsolute via a beamed MIR.

}

void SaturnInfo::new_connection_result()
{
  // Called by SunAbsolute upon receiving new_connection_request().
  // Beamed back to host that sent the original request.

  // unstream and set data
  // signal condition
}

void SaturnInfo::handle_login(TMessage* msg)
{
  // Called by SunAbsoulte upon receiving login request.


}

void SaturnInfo::login_result()
{
  // Called by SunAbsolute upon finalizing login procedure of a new MEE.
  // Beamed back to host that sent the original request.

  // unstream data
  // signal condition
}

*/

/**************************************************************************/
/**************************************************************************/

void SaturnInfo::AddMoon(SaturnInfo* moon)
{
  // blabla ...
  // moon->create_lists
}

void SaturnInfo::AddEye(EyeInfo* eye)
{

}

/**************************************************************************/

void SaturnInfo::ReceiveBeamResult(UInt_t req_handle)
{
  // Should be called via S_ and passed as beam to beam result
  // requestor (a SaturnInfo).

  static const Exc_t _eh("SaturnInfo::ReceiveBeamResult ");

  ZMIR* mir = assert_MIR_presence(_eh);

  mSaturn->handle_mir_result(req_handle, mir);
}

/**************************************************************************/

void SaturnInfo::TellAverages(Int_t sample_time)
{
  // Sort-of-a demo for Beam Result facility.
  // Executes in a detached thread.

  static const Exc_t _eh("SaturnInfo::TellAverages ");

  assert_MIR_presence(_eh, ZGlass::MC_IsBeam | ZGlass::MC_HasResultReq);

  CpuInfo_t cpu; gSystem->GetCpuInfo(&cpu, sample_time);

  TBufferFile ret(TBuffer::kWrite);
  ret << cpu.fLoad1m << cpu.fLoad5m << cpu.fLoad15m;
  mSaturn->ShootMIRResult(ret);
}

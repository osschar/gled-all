// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "EyeInfo.h"
#include <Net/Ray.h>
#include <Stones/ZMIR.h>
#include <Ephra/Saturn.h>

ClassImp(EyeInfo)

EyeInfo::EyeInfo(const Text_t* n, const Text_t* t) : 
  ZMirEmittingEntity(n,t)
{
  hSocket = 0; mMaster = 0;
}

/**************************************************************************/

void EyeInfo::Message(const Text_t* s)
{
  ZMIR* mir = get_MIR();
  if(mir == 0) return;
  TextMessage tm(mir->Caller, TextMessage::TM_Message, s);
  mSaturn->DeliverTextMessage(this, tm);
}

void EyeInfo::Warning(const Text_t* s)
{
  ZMIR* mir = get_MIR();
  if(mir == 0) return;
  TextMessage tm(mir->Caller, TextMessage::TM_Warning, s);
  mSaturn->DeliverTextMessage(this, tm);
}

void EyeInfo::Error(const Text_t* s)
{
  ZMIR* mir = get_MIR();
  if(mir == 0) return;
  TextMessage tm(mir->Caller, TextMessage::TM_Error, s);
  mSaturn->DeliverTextMessage(this, tm);
}

/**************************************************************************/

#include "EyeInfo.c7"

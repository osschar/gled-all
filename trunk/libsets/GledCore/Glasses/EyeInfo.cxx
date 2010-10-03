// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "EyeInfo.h"
#include "EyeInfo.c7"

#include <Eye/Ray.h>
#include <Stones/ZMIR.h>
#include <Ephra/Saturn.h>

ClassImp(EyeInfo);

EyeInfo::EyeInfo(const Text_t* n, const Text_t* t) :
  ZMirEmittingEntity(n,t)
{
  hSocket = 0;
  hEye = 0;
}

/**************************************************************************/

void EyeInfo::Message(const TString& s)
{
  ZMIR* mir = get_MIR();
  if(mir == 0) return;
  auto_ptr<TextMessage> tm(new TextMessage(mir->fCaller, ISmessage, s));
  mSaturn->DeliverTextMessage(this, tm);
}

void EyeInfo::Warning(const TString& s)
{
  ZMIR* mir = get_MIR();
  if(mir == 0) return;
  auto_ptr<TextMessage> tm(new TextMessage(mir->fCaller, ISwarning, s));
  mSaturn->DeliverTextMessage(this, tm);
}

void EyeInfo::Error(const TString& s)
{
  ZMIR* mir = get_MIR();
  if(mir == 0) return;
  auto_ptr<TextMessage> tm(new TextMessage(mir->fCaller, ISerror, s));
  mSaturn->DeliverTextMessage(this, tm);
}

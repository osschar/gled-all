// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "EyeInfo.h"
#include "EyeInfo.c7"

#include <Ephra/EyeInfoVector.h>
#include <Stones/ZMIR.h>
#include <Ephra/Saturn.h>

ClassImp(EyeInfo);

EyeInfo::EyeInfo(const Text_t* n, const Text_t* t) :
  ZMirEmittingEntity(n,t),
  hSocket(0), hEye(0), hSelfEIV(0),
  hEyeMutex(GMutex::recursive)
{
  hSelfEIV = new EyeInfoVector(0);
  hSelfEIV->IncRefCnt();
  hSelfEIV->push_back(this);
}

EyeInfo::~EyeInfo()
{
  hSelfEIV->DecRefCnt();
}

void EyeInfo::set_eye(Eye* eye)
{
  // Called from Eye on instantiation / destruction

  static const Exc_t _eh("EyeInfo::set_eye ");

  GMutexHolder _el(hEyeMutex);

  if (hEye == 0)
  {
    if (eye == 0)
      throw _eh + "Eye not set, called with zero again.";

    hEye = eye;
  }
  else
  {
    if (eye != 0)
      throw _eh + "Eye already set.";

    hEye = 0;
  }
}

/**************************************************************************/

void  EyeInfo::emit_text_message_ray(const TString& s, InfoStream_e type)
{
  ZMIR* mir = get_MIR();
  if (mir == 0) return;

  GMutexHolder _el(hEyeMutex);

  if (mSaturn->AcceptsRays() && hEye != 0)
  {
    auto_ptr<Ray> ray(Ray::PtrCtor(this, PRQN_text_message, mTimeStamp, FID()));
    ray->CustomBuffer() << mir->fCaller->GetSaturnID() << s << type;
    mSaturn->Shine(ray, hSelfEIV);
  }
}

void EyeInfo::Message(const TString& s)
{
  emit_text_message_ray(s, ISmessage);
}

void EyeInfo::Warning(const TString& s)
{
  emit_text_message_ray(s, ISwarning);
}

void EyeInfo::Error(const TString& s)
{
  emit_text_message_ray(s, ISerror);
}

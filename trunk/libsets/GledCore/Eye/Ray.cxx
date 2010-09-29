// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// Ray
//
// Ray contains stamping information. Rays are sent from Saturn to Eye.
// RayQN_e (Ray Quantum Number) lists known event types.
//

#include "Ray.h"
#include <Glasses/ZGlass.h>
#include <Glasses/AList.h>
#include <TBufferFile.h>

/**************************************************************************/

namespace RayNS
{
  void PutPTR(TBuffer& b, ZGlass*& p)
  {
    b.WriteFastArray((Char_t*)(&p), sizeof(void*));
  }
  void GetPTR(TBuffer& b, ZGlass*& p)
  {
    b.ReadFastArray((Char_t*)(&p), sizeof(void*));
  }

  void PutAnyPTR(TBuffer& b, void*& p)
  {
    b.WriteFastArray((Char_t*)(&p), sizeof(void*));
  }
  void GetAnyPTR(TBuffer& b, void*& p)
  {
    b.ReadFastArray((Char_t*)(&p), sizeof(void*));
  }
}

/**************************************************************************/
// Ray
/**************************************************************************/

Ray::~Ray()
{
  if (mCustomBuffer)
  {
    delete mCustomBuffer;
  }
}

/**************************************************************************/
// Ray - Writer side
/**************************************************************************/

void Ray::SetRefCnt(Int_t rc)
{
  mRefCnt = rc;
  if (mCustomBuffer)
  {
    mCustomBuffer->SetReadMode();
  }
}

TBuffer& Ray::CustomBuffer()
{
  assert (mCustomBuffer == 0);
  mCustomBuffer = new TBufferFile(TBuffer::kWrite, TBuffer::kMinimalSize);
  return *mCustomBuffer;
}

/**************************************************************************/
// Ray - Reader side
/**************************************************************************/

TBuffer& Ray::LockCustomBuffer()
{
  assert (mCustomBuffer != 0);
  mLock.Lock();
  mCustomBuffer->SetBufferOffset(0);
  return *mCustomBuffer;
}

void Ray::UnlockCustomBuffer()
{
  mLock.Unlock();
}

/**************************************************************************/
// Print-out / debug
/**************************************************************************/

const char* Ray::EventName() const
{
  using namespace RayNS;

  switch(fRQN)
  {
    case RQN_change:            return "change";
    case RQN_link_change:       return "link_change";

    case RQN_list_push_back:    return "list_push_back";
    case RQN_list_pop_back:     return "list_pop_back";
    case RQN_list_push_front:   return "list_push_front";
    case RQN_list_pop_front:    return "list_pop_front";
    case RQN_list_insert:       return "list_insert";
    case RQN_list_remove:       return "list_remove";
    case RQN_list_element_set:  return "list_element_set";
    case RQN_list_insert_label: return "list_insert_label";
    case RQN_list_remove_label: return "list_remove_label";
    case RQN_list_rebuild:      return "list_rebuild";
    case RQN_list_clear:        return "list_clear";

    case RQN_death:             return "death";
  }
  return "<unknown>";
}

void Ray::Dump(ostream& s) const
{
  s << EventName() <<" "<< fStamp << " ";
  s << fAlpha->GetSaturnID() <<", ("<< fFID.fLid <<","<< fFID.fCid <<")";
}

ostream& operator<<(ostream& s, const Ray& r)
{
  r.Dump(s);
  return s;
}


/**************************************************************************/
/**************************************************************************/
// TextMessage
/**************************************************************************/

void TextMessage::Streamer(TBuffer& buf)
{
  if(buf.IsReading()) {
    UShort_t t;
    RayNS::GetPTR(buf, fCaller);
    buf >> t >> fMessage;
    fType = (Type_e)t;
    ISdebug(9, GForm("TextMessage::Streamer Read  %u %d %s",
	       fCaller ? fCaller->GetSaturnID() : 0, fType, fMessage.Data()));
  } else {
    RayNS::PutPTR(buf, fCaller);
    buf << (UShort_t)fType << fMessage;
    ISdebug(9, GForm("TextMessage::Streamer Wrote %u %d %s",
	       fCaller ? fCaller->GetSaturnID() : 0, fType, fMessage.Data()));
  }
}

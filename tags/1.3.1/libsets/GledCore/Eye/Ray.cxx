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

namespace RayNS {

  void PutPTR(TBuffer& b, ZGlass*& p) {
    b.WriteFastArray((Char_t*)(&p), sizeof(void*));
  }
  void GetPTR(TBuffer& b, ZGlass*& p) {
    b.ReadFastArray((Char_t*)(&p), sizeof(void*));
  }

  void PutAnyPTR(TBuffer& b, void*& p) {
    b.WriteFastArray((Char_t*)(&p), sizeof(void*));
  }
  void GetAnyPTR(TBuffer& b, void*& p) {
    b.ReadFastArray((Char_t*)(&p), sizeof(void*));
  }
}

/**************************************************************************/
// Ray
/**************************************************************************/

Ray::~Ray()
{
  if(fRayBits & RB_CustomBuffer)
    delete fCustomBuffer;
}

/**************************************************************************/
// Ray - Writer side
/**************************************************************************/

void Ray::Write(TBuffer& b)
{
  using namespace RayNS;
  PutPTR(b, fAlpha);
  b << fRQN << fEyeBits;
  b << fStamp << fFID;
  b << fRayBits;
  if(fRayBits & RB_Beta)       PutPTR(b, fBeta);
  if(fRayBits & RB_Gamma)      PutPTR(b, fGamma);
  if(fRayBits & RB_BetaId)     b << fBetaId;
  if(fRayBits & RB_GammaId)    b << fGammaId;
  if(fRayBits & RB_BetaLabel)  b << fBetaLabel;
  if(fRayBits & RB_GammaLabel) b << fGammaLabel;
  if(fRayBits & RB_CustomBuffer) {
    b << fCustomLen;
    b.WriteFastArray(fCustomBuffer->Buffer(), fCustomLen);
  }
}

void Ray::SetCustomBuffer(TBuffer& b)
{
  fCustomLen = b.Length();
  fCustomBuffer = new TBufferFile(TBuffer::kWrite, fCustomLen, b.Buffer(), kTRUE);
  b.DetachBuffer();
  fCustomBuffer->SetBufferOffset(fCustomLen);
  fRayBits |= RB_CustomBuffer;
}

/**************************************************************************/
// Ray - Reader side
/**************************************************************************/

void Ray::Read(TBuffer& b)
{
  using namespace RayNS;
  b >> fRQN >> fEyeBits;
  b >> fStamp >> fFID;
  b >> fRayBits;
  if(fRayBits & RB_Beta)       GetPTR(b, fBeta);  else fBeta  = 0;
  if(fRayBits & RB_Gamma)      GetPTR(b, fGamma); else fGamma = 0;
  if(fRayBits & RB_BetaId)     b >> fBetaId;  else  fBetaId = -1;
  if(fRayBits & RB_GammaId)    b >> fGammaId; else fGammaId = -1;
  if(fRayBits & RB_BetaLabel)  b >> fBetaLabel;
  if(fRayBits & RB_GammaLabel) b >> fGammaLabel;
  if(fRayBits & RB_CustomBuffer) {
    b >> fCustomLen;
    fCustomBuffer = new TBufferFile(TBuffer::kRead, fCustomLen, b.Buffer() + b.Length(), kFALSE);
  } else {
    fCustomLen = 0; fCustomBuffer = 0;
  }
}

void Ray::ResetCustomBuffer()
{
  fCustomBuffer->SetBufferOffset(0);
}

/**************************************************************************/
// Print-out / debug
/**************************************************************************/

const char* Ray::EventName() const
{
  using namespace RayNS;

  switch(fRQN) {
  case RQN_change:	return "change";
  case RQN_link_change: return "link_change";

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

  case RQN_death:	return "death";
  }
  return "<unknown>";
}

void Ray::Dump(ostream& s) const {
  s << EventName() <<" "<< fStamp << " ";
  s << fAlpha->GetSaturnID() <<", ("<< fFID.fLid <<","<< fFID.fCid <<")";
}

ostream& operator<<(ostream& s, const Ray& r) {
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
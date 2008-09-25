// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Ray.h"
#include <Glasses/ZGlass.h>
#include <TBuffer.h>

// !!!!! should have caller field; now reused in message
// !!!!! could even provide ctor from mir (or sth)

void Ray::Streamer(TBuffer &buf)
{
  if(buf.IsReading()) {
    unsigned char uc;
    UInt_t caller=0;
    UInt_t a=0, b=0, g=0;
    buf >> uc;
    fEvent = (RayQN_e)uc;
    fStamp = 0;
    switch(fEvent) {
    case RQN_change:
    case RQN_link_change:  GetSLC(buf); buf >> a; break;
    case RQN_list_add:     GetSLC(buf); buf >> a >> b >> g; break;
    case RQN_list_remove:  GetSLC(buf); buf >> a >> b; break;
    case RQN_list_rebuild: GetSLC(buf); buf >> a; break;
    case RQN_birth:	   GetSLC(buf); buf >> a; break;
    case RQN_death:	   GetSLC(buf); buf >> a; break;
    case RQN_message:
    case RQN_error:	   buf >> caller >> fMessage; break;
    case RQN_apocalypse:   break;
    }
    fCaller = (ZGlass*)caller;
    fAlpha = (ZGlass*)a; fBeta = (ZGlass*)b; fGamma = (ZGlass*)g;
    ISdebug(9, GForm("Ray::Streamer Read\t%u\t%u", fAlpha ? fAlpha->GetSaturnID() : 0, fStamp));
  } else {
    buf << ((unsigned char)fEvent);
    switch(fEvent) {
    case RQN_change:
    case RQN_link_change:  PutSLC(buf); buf << UInt_t(fAlpha); break;
    case RQN_list_add:     PutSLC(buf); buf << UInt_t(fAlpha) << UInt_t(fBeta) << UInt_t(fGamma); break;
    case RQN_list_remove:  PutSLC(buf); buf << UInt_t(fAlpha) << UInt_t(fBeta); break;
    case RQN_list_rebuild: PutSLC(buf); buf << UInt_t(fAlpha); break;
    case RQN_birth:	   PutSLC(buf); buf << UInt_t(fAlpha); break;
    case RQN_death:	   PutSLC(buf); buf << UInt_t(fAlpha); break;
    case RQN_message:
    case RQN_error:        buf << UInt_t(fCaller) << fMessage; break;
    case RQN_apocalypse:   break;
    }
    ISdebug(9, GForm("Ray::Streamer Wrote\t%u\t%u", fAlpha ? fAlpha->GetSaturnID() : 0, fStamp));
  }
}

void Ray::PutSLC(TBuffer &buf) { buf << fStamp << fLibID << fClassID; }

void Ray::GetSLC(TBuffer &buf) { buf >> fStamp >> fLibID >> fClassID; }

/**************************************************************************/

const char* Ray::EventName() const
{
  switch(fEvent) {
  case RQN_change:	return "change";
  case RQN_link_change: return "link_change";
  case RQN_list_add:    return "list_add";
  case RQN_list_remove: return "list_remove";
  case RQN_list_rebuild:return "list_rebuild";
  case RQN_birth:	return "birth";
  case RQN_death:	return "death";
  case RQN_message:     return "message";
  case RQN_error:       return "error";
  case RQN_apocalypse:  return "apocalypse";
  }
  return "<unknown>";
}

void Ray::Dump(ostream& s) const {
  s << EventName() <<"\t"<< fStamp << "\t";
  switch(fEvent) {
  case RQN_change:
  case RQN_link_change: s << fAlpha->GetSaturnID() <<","<< fLibID <<","<< fClassID; break;
  case RQN_list_add:    s << fAlpha->GetSaturnID() <<","<< fBeta->GetSaturnID() <<","<< (fGamma ? fGamma->GetSaturnID() : 0); break;
  case RQN_list_remove: s << fAlpha->GetSaturnID() <<","<< fBeta->GetSaturnID(); break;
  case RQN_list_rebuild:s << fAlpha->GetSaturnID(); break;
  case RQN_birth:	s << fAlpha->GetSaturnID(); break;
  case RQN_death:	s << fAlpha->GetSaturnID(); break;
  case RQN_message:
  case RQN_error:       s << fCaller->GetSaturnID() <<","<< fMessage; break;
  case RQN_apocalypse:  break;
  }
}

ostream& operator<<(ostream& s, const Ray& r) {
  r.Dump(s);
  return s;
}

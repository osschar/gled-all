// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//________________________________________________________________________
//
// ZMIR
//
// Covering structure for streaming/routing of MIRs.
// Message type defaults to MT_Flare.
// By calling SetRecipient the message type is transmuted to MT_Beam.
// Direction is used for determination of routing/exec action in the Saturn.

#include "ZMIR.h"
#include <Ephra/Saturn.h>
#include <Glasses/SaturnInfo.h>
#include <Gled/GledNS.h>
#include <TMessage.h>

ClassImp(ZMIR)

/**************************************************************************/

void ZMIR::_init() {
  Caller = 0; Recipient = 0;
  Alpha = Beta = Gamma = 0;
}

/**************************************************************************/

ZMIR::ZMIR(ID_t a, ID_t b, ID_t g, ID_t c) :
  CallerID(c), RecipientID(0), AlphaID(a), BetaID(b), GammaID(g),
  Message(0), Direction(D_Unknown)
{
  _init();
  Message = new TMessage(GledNS::MT_Flare);
  Streamer(*Message);
}


ZMIR::ZMIR(TMessage* m) :
  Message(m), Direction(D_Unknown)
{
  _init();
  assert(Message->IsReading());
  Streamer(*m);
}

ZMIR::~ZMIR() {
  delete Message;
}

/**************************************************************************/

void ZMIR::Demangle(Saturn* s) throw(string)
{
  string eb("ZMIR::Demangle failed for ");
  Alpha = s->DemangleID(AlphaID);
  if(!Alpha) throw(eb + "Alpha");

  if(BetaID) {
    Beta = s->DemangleID(BetaID);
    if(!Beta) throw(eb + "Beta");
  } else {
    Beta = 0;
  }
  if(GammaID) {
    Gamma = s->DemangleID(GammaID);
    if(!Gamma) throw(eb + "Gamma");
  } else {
    Gamma = 0;
  }
  if(CallerID) {
    Caller = s->DemangleID(CallerID);
    if(!Caller) throw(eb + "Caller");
  } else {
    Caller = 0;
  }
  if(RecipientID) {
    Recipient = dynamic_cast<SaturnInfo*>(s->DemangleID(RecipientID));
    if(!Recipient) throw(eb + "Recipient");
  } else {
    Recipient = 0;
  }
}

/**************************************************************************/

void ZMIR::Rewind()
{
  // Sets Read mode and position to the beginning of the buffer.
  Message->SetReadMode();
  Message->SetBufferOffset(2*sizeof(UInt_t));
}

void ZMIR::RewindToMIR()
{
  // Sets Read mode and position to just after the Context header.
  Message->SetReadMode();
  Message->SetBufferOffset(2*sizeof(UInt_t) + 5*sizeof(ID_t) + sizeof(Int_t));
}

void ZMIR::SetCaller(ZGlass* caller)
{
  assert(Message!=0);
  if(Message->IsWriting()) {
    Caller = caller;
    CallerID = caller ? caller->GetSaturnID() : 0;
    Int_t pos = Message->Length();
    Message->SetBufferOffset(2*sizeof(UInt_t));
    *Message << CallerID;
    Message->SetBufferOffset(pos);
  } else {
    CallerID = caller ? caller->GetSaturnID() : 0;
    Int_t pos = Message->Length();
    Message->SetBufferOffset(2*sizeof(UInt_t));
    Message->SetWriteMode();
    *Message << CallerID;
    Message->SetBufferOffset(pos);
    Message->SetReadMode();
  }
}

void ZMIR::SetRecipient(SaturnInfo* recipient)
{
  assert(Message!=0);

  Message->SetWhat(GledNS::MT_Beam);
  Recipient = recipient;
  RecipientID = recipient->GetSaturnID();
  Int_t pos = Message->Length();
  Message->SetBufferOffset(2*sizeof(UInt_t) + sizeof(ID_t));
  *Message << RecipientID;
  Message->SetBufferOffset(pos);
}

void ZMIR::ClearRecipient()
{
  assert(Message!=0);

  Message->SetWhat(GledNS::MT_Flare);
  Recipient = 0;
  RecipientID = 0;
  Int_t pos = Message->Length();
  Message->SetBufferOffset(2*sizeof(UInt_t) + sizeof(ID_t));
  *Message << ID_t(0);
  Message->SetBufferOffset(pos);
}

/**************************************************************************/

void ZMIR::Streamer(TBuffer &R__b)
{
   // Stream an object of class ZMIR.

   if (R__b.IsReading()) {
      R__b >> CallerID;
      R__b >> RecipientID;
      R__b >> AlphaID;
      R__b >> BetaID;
      R__b >> GammaID;
      R__b >> UserFlags;
   } else {
      R__b << CallerID;
      R__b << RecipientID;
      R__b << AlphaID;
      R__b << BetaID;
      R__b << GammaID;
      R__b << UserFlags;
   }
}

/**************************************************************************/

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//________________________________________________________________________
//
// ZMIR
//
// Covering structure for streaming/routing of MIRs.
// Message type defaults to MT_Flare.
// By calling SetRecipient the message type is transmuted to MT_Beam.
//
// Direction is used for determination of routing/exec action in the Saturn.
// SuppressFlareBroadcast can be set during MIR execution on the sun-saturn
// to prevent broadcasting of the MIR to moons.

#include "ZMIR.h"
#include <Glasses/SaturnInfo.h>
#include <Gled/GledNS.h>

/**************************************************************************/
// ZMIR
/**************************************************************************/

ClassImp(ZMIR)

/**************************************************************************/

namespace {

  const Int_t ROOT_Header_Length = 2*sizeof(UInt_t); // len + type

  const Int_t Routing_Header_Min_Length = ROOT_Header_Length +
  sizeof(UChar_t) + sizeof(ID_t) + sizeof(ID_t); // MirBits, Caller, A

  const Int_t Fixed_Header_Length = ROOT_Header_Length +
    sizeof(UChar_t) + 4*sizeof(ID_t) +               // MirBits, Caller, A,B,G
    sizeof(LID_t) + sizeof(CID_t) + sizeof(MID_t);   // full method identity

  const Int_t MirBits_Length	= sizeof(UChar_t);
  const Int_t Caller_Length     = sizeof(ID_t);
  const Int_t Recipient_Length  = sizeof(ID_t);
  const Int_t ResultReq_Length  = sizeof(ID_t) + sizeof(UInt_t);
  const Int_t Max_Header_Length = Fixed_Header_Length + Recipient_Length + ResultReq_Length;

  const Int_t Caller_Offset    = ROOT_Header_Length + MirBits_Length;
  const Int_t Recipient_Offset = Caller_Offset      + Caller_Length;
  const Int_t ResultReq_Offset = Recipient_Offset   + Recipient_Length;
};

/**************************************************************************/

void ZMIR::_init() {
  // Set demangled info to null
  Caller = 0; Recipient = ResultRecipient = 0;
  Alpha = Beta = Gamma = 0;
}

/**************************************************************************/
 
ZMIR::ZMIR(ID_t a, ID_t b, ID_t g) :
  TMessage(GledNS::MT_Flare),
  Direction(D_Unknown), SuppressFlareBroadcast(false),
  MirBits(0), CallerID(0),
  RecipientID(0), ResultRecipientID(0), ResultReqHandle(0),
  AlphaID(a), BetaID(b), GammaID(g)
{
  _init();
  fTrueBuffer = 0;
  SetBufferOffset(Max_Header_Length);
}

ZMIR::ZMIR(ZGlass* a, ZGlass* b, ZGlass* g) :
  TMessage(GledNS::MT_Flare),
  Direction(D_Unknown), SuppressFlareBroadcast(false),
  MirBits(0), CallerID(0),
  RecipientID(0), ResultRecipientID(0), ResultReqHandle(0),
  Alpha(a), Beta(b), Gamma(g)
{
  AlphaID = a ? a->GetSaturnID() : 0;
  BetaID  = b ? b->GetSaturnID() : 0;
  GammaID = g ? g->GetSaturnID() : 0;
  fTrueBuffer = 0;
  SetBufferOffset(Max_Header_Length);
  Caller = 0; Recipient = ResultRecipient = 0;
}


ZMIR::ZMIR(TMessage*& m) :
  TMessage(m->Buffer(), m->BufferSize()),
  Direction(D_Unknown),
  SuppressFlareBroadcast(false), RequiresResult(false)
{
  m->DetachBuffer(); delete m; m = 0;
  _init();
  fTrueBuffer = 0;
}

ZMIR::ZMIR(void* buf, Int_t size) :
  TMessage(buf,size),
  Direction(D_Unknown),
  SuppressFlareBroadcast(false), RequiresResult(false)
{
  _init();
  fTrueBuffer = 0;
}

ZMIR::~ZMIR() {
  if(fTrueBuffer) fBuffer = fTrueBuffer;
}

/**************************************************************************/

Int_t ZMIR::HeaderLength()
{
  Int_t r = Fixed_Header_Length;
  if(MirBits & MB_HasRecipient) r += sizeof(ID_t);
  if(MirBits & MB_HasResultReq) r += ResultReq_Length;
  return r;
}

Int_t ZMIR::RoutingHeaderLength()
{
  Int_t r = Routing_Header_Min_Length;
  if(MirBits & MB_HasRecipient) r += sizeof(ID_t);
  if(MirBits & MB_HasResultReq) r += ResultReq_Length;
  return r;
}

void ZMIR::WriteHeader()
{
  // Writes a complete MIR header, including the message type. If
  // Recipient and ResultReq fields are not used, the position of
  // fBuffer is displaced for the appropriate amount.
  // Also ... sets fBufSize to the current buffer position.
  // Do not add further data after this method has been called!

  if(IsWriting()) {
    if(MirBits & MB_HeaderWritten) return;
    MirBits |= MB_HeaderWritten;

    fTrueBuffer = fBuffer;
    fBuffer += Max_Header_Length - HeaderLength();

    fBufSize  = Length();
    SetBufferOffset(sizeof(UInt_t));

    TBuffer& b = *this;
    b << What() << MirBits << CallerID;
    if(MirBits & MB_HasRecipient) b << RecipientID;
    if(MirBits & MB_HasResultReq) b << ResultRecipientID << ResultReqHandle;
    b << AlphaID << BetaID << GammaID;
    b << Lid << Cid << Mid;

    SetBufferOffset(fBufSize);
  }
}

void ZMIR::ReadRoutingHeader()
{
  assert(IsReading()); // or sth ...
  TBuffer& b = *this;
  b >> MirBits >> CallerID;
  if(MirBits & MB_HasRecipient) b >> RecipientID;
  if(MirBits & MB_HasResultReq) b >> ResultRecipientID >> ResultReqHandle;
  b >> AlphaID;
}

void ZMIR::ReadExecHeader()
{
  assert(IsReading()); // or sth else ...
  //if(!IsReading()) { ZGlass* p=0; p->GetName(); }
  TBuffer& b = *this;
  b >> BetaID >> GammaID;
  b >> Lid >> Cid >> Mid;
}

void ZMIR::RewindToData()
{
  // Sets Read mode and position to just after the Context header.

  SetReadMode();
  SetBufferOffset(HeaderLength());
}

void ZMIR::RewindToExecHeader()
{
  // Sets Read mode and position to just after the Context header.

  SetReadMode();
  SetBufferOffset(RoutingHeaderLength());
}

/**************************************************************************/

namespace {
  const TString demangle_eh("ZMIR::Demangle failed for ");
}

void ZMIR::Demangle(An_ID_Demangler* s) throw(TString)
{
  Caller = dynamic_cast<ZMirEmittingEntity*>(s->DemangleID(CallerID));
  if(!Caller) throw(demangle_eh + GForm("Caller(id=%d)",CallerID));

  // Recipient & Result recipient separated. Called by Saturn when appropriate.

  Alpha = s->DemangleID(AlphaID);
  if(!Alpha) throw(demangle_eh + GForm("Alpha [%d]", AlphaID));
  if(BetaID) {
    Beta = s->DemangleID(BetaID);
    if(!Beta) throw(demangle_eh + GForm("Beta", BetaID));
  }
  if(GammaID) {
    Gamma = s->DemangleID(GammaID);
    if(!Gamma) throw(demangle_eh + GForm("Gamma", GammaID));
  }
}

void ZMIR::DemangleRecipient(An_ID_Demangler* s) throw(TString)
{
  if(MirBits & MB_HasRecipient) {
    Recipient = dynamic_cast<SaturnInfo*>(s->DemangleID(RecipientID));
    if(!Recipient) throw(demangle_eh + GForm("Recipient", RecipientID));
  }
}

void ZMIR::DemangleResultRecipient(An_ID_Demangler* s) throw(TString)
{
  if(MirBits & MB_HasResultReq) {
    ResultRecipient = dynamic_cast<SaturnInfo*>(s->DemangleID(ResultRecipientID));
    if(!ResultRecipient) throw(demangle_eh + "ResultRecipient");
  }
}

/**************************************************************************/

void ZMIR::SetCaller(ZMirEmittingEntity* caller)
{
  Caller = caller;
  if(IsWriting()) {
    CallerID = caller ? caller->GetSaturnID() : 0;
  } else {
    // Called by Saturn to set Caller identity for MIRs coming from
    // Eyes and being posted by threads.
    CallerID = caller ? caller->GetSaturnID() : 0;
    Int_t pos = Length();
    SetBufferOffset(Caller_Offset);
    *this << CallerID;
    SetBufferOffset(pos);
  }
  
}

void ZMIR::SetRecipient(SaturnInfo* recipient)
{
  // Can be called for MIR in write mode or for MIR in read mode
  // if it is already a Beam.
  // recipient == 0 means local invocation.

  RecipientID = recipient ? recipient->GetSaturnID() : 0;
  if(IsReading()) {
    assert(MirBits | MB_HasRecipient);
    Recipient = recipient;
    Int_t pos = Length();
    SetBufferOffset(Recipient_Offset);
    *this << RecipientID;
    SetBufferOffset(pos);
  } else {
    MirBits |= MB_HasRecipient;
    SetWhat(GledNS::MT_Beam);
  }
}

void ZMIR::ClearRecipient()
{
  // Can only be called for MIRs in read mode. MIR is transmuted into Flare,
  // but the HasRecipient flag stays on.

  assert(IsReading() && (MirBits | MB_HasRecipient));
  RecipientID = 0;
  Recipient   = 0;

  SetWhat(GledNS::MT_Flare);
  Int_t pos = Length();
  SetBufferOffset(Recipient_Offset);
  *this << RecipientID;
  SetBufferOffset(pos);
}

void ZMIR::SetResultReq(SaturnInfo* r_recipient, UInt_t r_handle)
{
  // Sets result request information of the MIR.
  // Beam results can also carry arbitrary streamed information.

  assert(IsWriting());

  MirBits |= MB_HasResultReq;
  ResultRecipientID = r_recipient->GetSaturnID();
  ResultReqHandle   = r_handle;
}

void ZMIR::SetDetachedExe(bool multix)
{
    assert(IsWriting());

    MirBits |= (multix) ? MB_DetachedExe | MB_MultixDetachedExe :
                          MB_DetachedExe;
    
}

/**************************************************************************/

void ZMIR::CopyToBuffer(TBuffer& b)
{
  // Copies contents of Message from current position until end.

  b.WriteFastArray(fBufCur, fBufSize - Length());
}

void ZMIR::AppendBuffer(TBuffer& b)
{
  // Appends contents of b to the MIR.

  WriteFastArray(b.Buffer(), b.Length());
}

/**************************************************************************/

void ZMIR::ChainMIR(ZMIR* mir)
{
  // Appends mir to the end of *this.

  MirBits |= MB_HasChainedMIR;
  mir->WriteHeader();
  mir->SetBufferOffset(0);
  mir->SetReadMode();
  mir->CopyToBuffer(*this);
}

ZMIR* ZMIR::UnchainMIR(An_ID_Demangler* s)
{
  // Creates a secondary mir from current position onwards.
  // This means that *this must be read to its end. Relevant if *this
  // uses custom-buffer.

  assert(HasChainedMIR());
  ZMIR* mir = new ZMIR(Buffer() + Length(), BufferSize() - Length());
  mir->ResetBit(TBuffer::kIsOwner);
  mir->SetCaller(Caller);
  mir->ReadRoutingHeader();
  mir->ReadExecHeader();
  if(s != 0)
    mir->Demangle(s);
  return mir;
}

/**************************************************************************/
// ZMIR_Result_Report
/**************************************************************************/

//______________________________________________________________________
// ZMIR_Result_Report
//

ClassImp(ZMIR_Result_Report)

const char* ZMIR_Result_Report::GenError(bool report_buffer)
{
  if(report_buffer) {
    int buflen = HasResult() ? BufferSize() : -1;
    return GForm("ZMIR_RR(buflen=%d, exc=\"%s\")", buflen, Exception.Data());
  } else {
    return GForm("ZMIR_RR(exc=\"%s\")", Exception.Data());
  }
}

// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZMIR.h"
#include <Glasses/SaturnInfo.h>
#include <Gled/GledNS.h>

/**************************************************************************/
// ZMIR
/**************************************************************************/

//________________________________________________________________________
//
// Covering structure for streaming/routing of MIRs.
// Message type defaults to MT_Flare.
// By calling SetRecipient the message type is transmuted to MT_Beam.
//
// Direction is used for determination of routing/exec action in the Saturn.
// SuppressFlareBroadcast can be set during MIR execution on the sun-saturn
// to prevent broadcasting of the MIR to moons.

ClassImp(ZMIR);

/**************************************************************************/

namespace
{
  const Int_t sROOT_Header_Length = 2*sizeof(UInt_t); // len + type

  const Int_t sRouting_Header_Min_Length = sROOT_Header_Length +
  sizeof(UChar_t) + sizeof(ID_t) + sizeof(ID_t); // MirBits, Caller, A

  const Int_t sFixed_Header_Length = sROOT_Header_Length +
    sizeof(UChar_t) + 4*sizeof(ID_t)  +                // MirBits, Caller,A,B,G
    sizeof(LID_t)   +   sizeof(CID_t) + sizeof(MID_t); // full method id

  const Int_t sMirBits_Length    = sizeof(UChar_t);
  const Int_t sCaller_Length     = sizeof(ID_t);
  const Int_t sRecipient_Length  = sizeof(ID_t);
  const Int_t sResultReq_Length  = sizeof(ID_t) + sizeof(UInt_t);
  const Int_t sMax_Header_Length = sFixed_Header_Length + sRecipient_Length + sResultReq_Length;

  const Int_t sCaller_Offset    = sROOT_Header_Length + sMirBits_Length;
  const Int_t sRecipient_Offset = sCaller_Offset      + sCaller_Length;
  const Int_t sResultReq_Offset = sRecipient_Offset   + sRecipient_Length;
};

/**************************************************************************/

void ZMIR::_init()
{
  // Set demangled info to null
  fCaller = 0; fRecipient = fResultRecipient = 0;
  fAlpha = fBeta = fGamma = 0;
}

/**************************************************************************/

ZMIR::ZMIR(ID_t a, ID_t b, ID_t g) :
  TMessage(GledNS::MT_Flare),
  fDirection(D_Unknown), fSuppressFlareBroadcast(false), fRequiresResult(false),
  fMirBits(0), fCallerID(0),
  fRecipientID(0), fResultRecipientID(0), fResultReqHandle(0),
  fAlphaID(a), fBetaID(b), fGammaID(g)
{
  _init();
  fTrueBuffer = 0;
  SetBufferOffset(sMax_Header_Length);
}

ZMIR::ZMIR(ZGlass* a, ZGlass* b, ZGlass* g) :
  TMessage(GledNS::MT_Flare),
  fDirection(D_Unknown), fSuppressFlareBroadcast(false), fRequiresResult(false) ,
  fMirBits(0), fCallerID(0),
  fRecipientID(0), fResultRecipientID(0), fResultReqHandle(0),
  fAlpha(a), fBeta(b), fGamma(g)
{
  fAlphaID = a ? a->GetSaturnID() : 0;
  fBetaID  = b ? b->GetSaturnID() : 0;
  fGammaID = g ? g->GetSaturnID() : 0;
  fTrueBuffer = 0;
  SetBufferOffset(sMax_Header_Length);
  fCaller = 0; fRecipient = fResultRecipient = 0;
}


ZMIR::ZMIR(TMessage*& m) :
  TMessage(m->Buffer(), m->BufferSize()),
  fDirection(D_Unknown),
  fSuppressFlareBroadcast(false), fRequiresResult(false)
{
  m->DetachBuffer(); delete m; m = 0;
  _init();
  fTrueBuffer = 0;
}

ZMIR::ZMIR(void* buf, Int_t size) :
  TMessage(buf,size),
  fDirection(D_Unknown),
  fSuppressFlareBroadcast(false), fRequiresResult(false)
{
  _init();
  fTrueBuffer = 0;
}

ZMIR::~ZMIR()
{
  if(fTrueBuffer) fBuffer = fTrueBuffer;
}

/**************************************************************************/

Int_t ZMIR::HeaderLength()
{
  Int_t r = sFixed_Header_Length;
  if(fMirBits & MB_HasRecipient) r += sizeof(ID_t);
  if(fMirBits & MB_HasResultReq) r += sResultReq_Length;
  return r;
}

Int_t ZMIR::RoutingHeaderLength()
{
  Int_t r = sRouting_Header_Min_Length;
  if(fMirBits & MB_HasRecipient) r += sizeof(ID_t);
  if(fMirBits & MB_HasResultReq) r += sResultReq_Length;
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
    if(fMirBits & MB_HeaderWritten) return;
    fMirBits |= MB_HeaderWritten;

    fTrueBuffer = fBuffer;
    fBuffer += sMax_Header_Length - HeaderLength();

    fBufSize  = Length();
    SetBufferOffset(sizeof(UInt_t));

    TBuffer& b = *this;
    b << What() << fMirBits << fCallerID;
    if(fMirBits & MB_HasRecipient) b << fRecipientID;
    if(fMirBits & MB_HasResultReq) b << fResultRecipientID << fResultReqHandle;
    b << fAlphaID << fBetaID << fGammaID;
    b << fLid << fCid << fMid;

    SetBufferOffset(fBufSize);
  }
}

void ZMIR::ReadRoutingHeader()
{
  assert(IsReading()); // or sth ...
  TBuffer& b = *this;
  b >> fMirBits >> fCallerID;
  if(fMirBits & MB_HasRecipient) b >> fRecipientID;
  if(fMirBits & MB_HasResultReq) b >> fResultRecipientID >> fResultReqHandle;
  b >> fAlphaID;
}

void ZMIR::ReadExecHeader()
{
  assert(IsReading()); // or sth else ...
  //if(!IsReading()) { ZGlass* p=0; p->GetName(); }
  TBuffer& b = *this;
  b >> fBetaID >> fGammaID;
  b >> fLid >> fCid >> fMid;
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

namespace
{
  const Exc_t demangle_eh("ZMIR::Demangle failed for ");
}

void ZMIR::Demangle(An_ID_Demangler* s) throw(Exc_t)
{
  fCaller = dynamic_cast<ZMirEmittingEntity*>(s->DemangleID(fCallerID));
  if(!fCaller) throw(demangle_eh + GForm("Caller(id=%d)", fCallerID));

  // Recipient & Result recipient separated. Called by Saturn when appropriate.

  fAlpha = s->DemangleID(fAlphaID);
  if(!fAlpha) throw(demangle_eh + GForm("Alpha [%d]", fAlphaID));
  if(fBetaID) {
    fBeta = s->DemangleID(fBetaID);
    if(!fBeta) throw(demangle_eh + GForm("Beta", fBetaID));
  }
  if(fGammaID) {
    fGamma = s->DemangleID(fGammaID);
    if(!fGamma) throw(demangle_eh + GForm("Gamma", fGammaID));
  }
}

void ZMIR::DemangleRecipient(An_ID_Demangler* s) throw(Exc_t)
{
  if(fMirBits & MB_HasRecipient) {
    fRecipient = dynamic_cast<SaturnInfo*>(s->DemangleID(fRecipientID));
    if(!fRecipient) throw(demangle_eh + GForm("Recipient", fRecipientID));
  }
}

void ZMIR::DemangleResultRecipient(An_ID_Demangler* s) throw(Exc_t)
{
  if(fMirBits & MB_HasResultReq) {
    fResultRecipient = dynamic_cast<SaturnInfo*>(s->DemangleID(fResultRecipientID));
    if(!fResultRecipient) throw(demangle_eh + "ResultRecipient");
  }
}

/**************************************************************************/

void ZMIR::SetCaller(ZMirEmittingEntity* caller)
{
  fCaller = caller;
  if(IsWriting()) {
    fCallerID = caller ? caller->GetSaturnID() : 0;
  } else {
    // Called by Saturn to set Caller identity for MIRs coming from
    // Eyes and being posted by threads.
    fCallerID = caller ? caller->GetSaturnID() : 0;
    Int_t pos = Length();
    SetBufferOffset(sCaller_Offset);
    *this << fCallerID;
    SetBufferOffset(pos);
  }
}

void ZMIR::SetRecipient(SaturnInfo* recipient)
{
  // Can be called for MIR in write mode or for MIR in read mode
  // if it is already a Beam.
  // recipient == 0 means local invocation.

  fRecipientID = recipient ? recipient->GetSaturnID() : 0;
  if(IsReading()) {
    assert(fMirBits | MB_HasRecipient);
    fRecipient = recipient;
    Int_t pos = Length();
    SetBufferOffset(sRecipient_Offset);
    *this << fRecipientID;
    SetBufferOffset(pos);
  } else {
    fMirBits |= MB_HasRecipient;
    SetWhat(GledNS::MT_Beam);
  }
}

void ZMIR::ClearRecipient()
{
  // Can only be called for MIRs in read mode. MIR is transmuted into Flare,
  // but the HasRecipient flag stays on.

  assert(IsReading() && (fMirBits | MB_HasRecipient));
  fRecipientID = 0;
  fRecipient   = 0;

  SetWhat(GledNS::MT_Flare);
  Int_t pos = Length();
  SetBufferOffset(sRecipient_Offset);
  *this << fRecipientID;
  SetBufferOffset(pos);
}

void ZMIR::SetResultReq(SaturnInfo* r_recipient, UInt_t r_handle)
{
  // Sets result request information of the MIR.
  // Beam results can also carry arbitrary streamed information.

  assert(IsWriting());

  fMirBits |= MB_HasResultReq;
  fResultRecipientID = r_recipient->GetSaturnID();
  fResultReqHandle   = r_handle;
}

void ZMIR::SetDetachedExe(bool multix)
{
    assert(IsWriting());

    fMirBits |= (multix) ? MB_DetachedExe | MB_MultixDetachedExe :
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

  fMirBits |= MB_HasChainedMIR;
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
  mir->SetCaller(fCaller);
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
//
// Structure for reporting result of MIR execution.
// It can contain a custom respone buffer.

ClassImp(ZMIR_Result_Report);

const char* ZMIR_Result_Report::GenError(bool report_buffer)
{
  if(report_buffer) {
    int buflen = HasResult() ? BufferSize() : -1;
    return GForm("ZMIR_RR(buflen=%d, exc=\"%s\")", buflen, fException.Data());
  } else {
    return GForm("ZMIR_RR(exc=\"%s\")", fException.Data());
  }
}

// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZMIR_H
#define Gled_ZMIR_H

#include <Gled/GledTypes.h>

class ZMirEmittingEntity;
class SaturnInfo;

#include <TMessage.h>

/*
  TBuffer annotations:
  int Length()		current pos
  SetBufferOffset(int) 	set pos
  Message header 2*UInt_t: length + message type
 */

/**************************************************************************/
// ZMIR
/**************************************************************************/

class ZMIR : public TMessage {

public:
  enum Direction_e { D_Unknown=0, D_Up, D_Down };

  enum MIR_Bits_e { MB_HeaderWritten = 1,
		    MB_HasRecipient  = 2,
		    MB_HasResultReq  = 4 };

private:
  void _init();

protected:
  char         *fTrueBuffer;	//!

public:
  Direction_e	Direction;	//!
  Bool_t	SuppressFlareBroadcast; //!

  UChar_t	MirBits;
  ID_t		CallerID;

  ID_t		RecipientID;		// Must be set via SetRecipient(id)
  ID_t		ResultRecipientID;	// \_ Must be set via
  UInt_t	ResultReqHandle;	// /    SetResultReq(id, handle)

  ID_t		AlphaID;
  ID_t		BetaID;
  ID_t		GammaID;

  LID_t		Lid;
  CID_t		Cid;
  MID_t		Mid;

  // Demangled SaturnIDs; used only on the receiving side.

  ZMirEmittingEntity*	Caller;		 //!
  SaturnInfo*		Recipient;	 //!
  SaturnInfo*		ResultRecipient; //!
  ZGlass*		Alpha;		 //!
  ZGlass*		Beta;		 //!
  ZGlass*		Gamma;		 //!

  ZMIR(ID_t a=0, ID_t b=0, ID_t g=0);
  ZMIR(ZGlass* a, ZGlass* b=0, ZGlass* g=0);
  ZMIR(TMessage*& m);
  virtual ~ZMIR();

  Bool_t HasRecipient()	{ return (MirBits & MB_HasRecipient); }
  Bool_t HasResultReq()	{ return (MirBits & MB_HasResultReq); }
  Bool_t IsFlare()      { return !HasRecipient(); }
  Bool_t IsBeam()       { return HasRecipient(); }

  Int_t HeaderLength();
  Int_t RoutingHeaderLength();
  void  WriteHeader();
  void  ReadRoutingHeader();
  void  ReadExecHeader();
  void  RewindToData();
  void  RewindToExecHeader();

  // virtual void Reset(); // restore buff from true buf;

  void Demangle(An_ID_Demangler* s) throw(string);
  void DemangleRecipient(An_ID_Demangler* s) throw(string);
  void DemangleResultRecipient(An_ID_Demangler* s) throw(string);

  void SetLCM_Ids(LID_t l, CID_t c, MID_t m) { Lid=l; Cid=c; Mid=m; }
  void SetCaller(ZMirEmittingEntity* caller);
  void SetRecipient(SaturnInfo* recipient);
  void ClearRecipient();
  void SetResultReq(SaturnInfo* r_recipient, UInt_t r_handle);

  void CopyToBuffer(TBuffer& b);
  void AppendBuffer(TBuffer& b);

  ClassDef(ZMIR, 0)
};

/**************************************************************************/
// ZMIR_Result_Report
/**************************************************************************/

class ZMIR_Result_Report : public TBuffer {
private:
   ZMIR_Result_Report(const ZMIR_Result_Report &); // not implemented
   void operator=(const ZMIR_Result_Report &);     // not implemented

public:
  enum Bits_e { B_HasException = 1, B_HasResult = 2};

  UChar_t	MirRRBits;
  TString	Exception;

  ZMIR_Result_Report() : TBuffer(TBuffer::kWrite) {}
  virtual ~ZMIR_Result_Report() {}

  Bool_t HasException()  { return MirRRBits & B_HasException; }
  Bool_t HasResult()     { return MirRRBits & B_HasResult; }
  Bool_t Exec_OK()       { return (MirRRBits & B_HasException) == 0; }
  Bool_t Flare_OK()      { return Exec_OK(); }
  Bool_t BeamResult_OK() { return (Exec_OK() && HasResult()); }

  const char* GenError(bool report_buffer=true);

  ClassDef(ZMIR_Result_Report, 0)
}; // endclass ZMIR_Result_Report

typedef ZMIR_Result_Report ZMIR_RR;

#endif

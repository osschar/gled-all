// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZMIR_H
#define Gled_ZMIR_H

#include <Gled/GledTypes.h>

class An_ID_Demangler;
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

class ZMIR : public TMessage
{
public:
  enum Direction_e { D_Unknown=0, D_Up, D_Down };

  enum MIR_Bits_e { MB_HeaderWritten      = 1,
		    MB_HasRecipient       = 2,
		    MB_HasResultReq       = 4,
                    MB_DetachedExe        = 8,
                    MB_MultixDetachedExe  = 16,
                    MB_HasChainedMIR      = 32
  };

private:
  void _init();

protected:
  char         *fTrueBuffer;	//!

public:
  Direction_e	fDirection;	         //!
  Bool_t	fSuppressFlareBroadcast; //!
  Bool_t	fRequiresResult;         //!

  UChar_t	fMirBits;
  ID_t		fCallerID;

  ID_t		fRecipientID;		// Must be set via SetRecipient(id)
  ID_t		fResultRecipientID;	// \_ Must be set via
  UInt_t	fResultReqHandle;	// /    SetResultReq(id, handle)

  ID_t		fAlphaID;
  ID_t		fBetaID;
  ID_t		fGammaID;

  LID_t		fLid;
  CID_t		fCid;
  MID_t		fMid;

  // Demangled SaturnIDs; used only on the receiving side.

  ZMirEmittingEntity*	fCaller;	  //!
  SaturnInfo*		fRecipient;	  //!
  SaturnInfo*		fResultRecipient; //!
  ZGlass*		fAlpha;		  //!
  ZGlass*		fBeta;		  //!
  ZGlass*		fGamma;		  //!

  ZMIR(ID_t a=0, ID_t b=0, ID_t g=0);
  ZMIR(ZGlass* a, ZGlass* b=0, ZGlass* g=0);
  ZMIR(TMessage*& m);
  ZMIR(void* buf, Int_t size);
  virtual ~ZMIR();

  Bool_t HasRecipient()	{ return (fMirBits & MB_HasRecipient); }
  Bool_t HasResultReq()	{ return (fMirBits & MB_HasResultReq); }
  Bool_t IsFlare()      { return !HasRecipient(); }
  Bool_t IsBeam()       { return HasRecipient(); }
  Bool_t ShouldExeDetached()   { return (fMirBits & MB_DetachedExe); }
  Bool_t IsDetachedExeMultix() { return (fMirBits & MB_MultixDetachedExe); }
  Bool_t HasChainedMIR()       { return (fMirBits & MB_HasChainedMIR); }

  Bool_t IsMatching(const FID_t& fid) const;
  Bool_t IsMatching(const FID_t& fid, MID_t mid) const;

  Int_t HeaderLength();
  Int_t RoutingHeaderLength();
  void  WriteHeader();
  void  ReadRoutingHeader();
  void  ReadExecHeader();
  void  RewindToData();
  void  RewindToExecHeader();

  // virtual void Reset(); // restore buff from true buf;

  void Demangle(An_ID_Demangler* s) throw(Exc_t);
  void DemangleRecipient(An_ID_Demangler* s) throw(Exc_t);
  void DemangleResultRecipient(An_ID_Demangler* s) throw(Exc_t);

  void SetLCM_Ids(LID_t l, CID_t c, MID_t m) { fLid=l; fCid=c; fMid=m; }
  void SetCaller(ZMirEmittingEntity* caller);
  void SetRecipient(SaturnInfo* recipient);
  void ClearRecipient();
  void SetResultReq(SaturnInfo* r_recipient, UInt_t r_handle);
  void SetDetachedExe(bool multix=false);

  void CopyToBuffer(TBuffer& b);
  void AppendBuffer(TBuffer& b);

  void  ChainMIR(ZMIR* mir);
  ZMIR* UnchainMIR(An_ID_Demangler* s=0);

  ClassDef(ZMIR, 0);
};

inline Bool_t ZMIR::IsMatching(const FID_t& fid) const
{
  return fid.fLid == fLid && fid.fCid == fCid;
}

inline Bool_t ZMIR::IsMatching(const FID_t& fid, MID_t mid) const
{
  return fid.fLid == fLid && fid.fCid == fCid && mid == fMid;
}

/**************************************************************************/
// ZMIR_Result_Report
/**************************************************************************/

class ZMIR_Result_Report : public TBufferFile
{
private:
   ZMIR_Result_Report(const ZMIR_Result_Report &); // not implemented
   void operator=(const ZMIR_Result_Report &);     // not implemented

public:
  enum Bits_e { B_HasException = 1, B_HasResult = 2 };

  UChar_t	fMirRRBits;
  TString	fException;

  ZMIR_Result_Report() : TBufferFile(TBuffer::kWrite) {}
  virtual ~ZMIR_Result_Report() {}

  Bool_t HasException()  { return  fMirRRBits & B_HasException; }
  Bool_t HasResult()     { return  fMirRRBits & B_HasResult; }
  Bool_t Exec_OK()       { return (fMirRRBits & B_HasException) == 0; }
  Bool_t Flare_OK()      { return  Exec_OK(); }
  Bool_t BeamResult_OK() { return (Exec_OK() && HasResult()); }

  const char* GenError(bool report_buffer=true);

  ClassDef(ZMIR_Result_Report, 0);
}; // endclass ZMIR_Result_Report

typedef ZMIR_Result_Report ZMIR_RR;

#endif

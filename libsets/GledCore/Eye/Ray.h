// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Ray_H
#define GledCore_Ray_H

// Includes
#include <Gled/GledTypes.h>
#include <Gled/GSpinLock.h>

namespace OptoStructs
{
  class ZGlassImg;
}

//==============================================================================

class Ray;
class TextMessage;
class EyeCommand;

namespace RayNS
{
  // Saturn -> Eye communication
  enum SaturnToEyeMessageTypes
  {
    MT_Ray             = 12050,	// Lens changes
    MT_TextMessage,             // Messages, Errors, Exceptions
    MT_EyeCommand               // Commands for the Eye
  };

  struct SaturnToEyeEnvelope
  {
    Int_t          fType;
    union
    {
      Ray         *fRay;
      TextMessage *fTextMessage;
      EyeCommand  *fEyeCommand;
    };

    SaturnToEyeEnvelope() {}
    SaturnToEyeEnvelope(Ray* x)         : fType(MT_Ray), fRay(x) {}
    SaturnToEyeEnvelope(TextMessage* x) : fType(MT_TextMessage), fTextMessage(x) {}
    SaturnToEyeEnvelope(EyeCommand* x)  : fType(MT_EyeCommand), fEyeCommand(x) {}
  };


  enum RayQN_e
  {
    RQN_death,
    RQN_change,
    RQN_link_change,
    RQN_list_begin,
    RQN_list_push_back, RQN_list_pop_back,
    RQN_list_push_front, RQN_list_pop_front,
    RQN_list_insert, RQN_list_remove,
    RQN_list_element_set,
    RQN_list_insert_label, RQN_list_remove_label,
    RQN_list_rebuild, RQN_list_clear,
    RQN_list_end,
    // Non-used; after that individual glasses/views can do anything.
    RQN_user_0 = 64,
    RQN_user_1, RQN_user_2, RQN_user_3, RQN_user_4, RQN_user_5
  };
}


//==============================================================================
// Ray
//==============================================================================

class Ray
{
public:

  enum EyeBits_e
  {
    EB_StructuralChange = 0x1
  };

  enum RayBits_e
  {
    RB_Beta         = 0x1,
    RB_Gamma        = 0x2,
    RB_BetaId       = 0x4,
    RB_GammaId      = 0x8,
    RB_BetaLabel    = 0x10,
    RB_GammaLabel   = 0x20
  };

protected:
  TBuffer*	mCustomBuffer;
  GSpinLock     mLock;
  Int_t         mRefCnt;

public:
  UChar_t	fRQN;		// ray quantum number
  UChar_t	fEyeBits;
  UChar_t	fRayBits;	//
  TimeStamp_t	fStamp;		// timestamp of alpaha
  FID_t		fFID;		// libset/glass of origin

  ZGlass*	fAlpha;

  // Optional part
  ZGlass*	fBeta;
  ZGlass*	fGamma;
  Int_t		fBetaId;
  Int_t		fGammaId;
  TString	fBetaLabel;
  TString      	fGammaLabel;

  //----------------------------------------------------------------------

  ~Ray();

  /************************************************************************/
  // Writer side
  /************************************************************************/

  Ray(ZGlass* a, UChar_t rqn, TimeStamp_t t, FID_t fid=FID_t(0,0), UChar_t eb=0) :
    mCustomBuffer(0), mRefCnt(0),
    fRQN(rqn), fEyeBits(eb), fRayBits(0), fStamp(t), fFID(fid), fAlpha(a)
  {}

  static Ray* PtrCtor(ZGlass* a, UChar_t rqn, TimeStamp_t t, FID_t fid, UChar_t eb=0)
  { return new Ray(a, rqn, t, fid, eb); }
  static Ray* PtrCtor(ZGlass* a, UChar_t rqn, TimeStamp_t t, UChar_t eb)
  { return new Ray(a, rqn, t, FID_t(0,0), eb); }

  void SetBeta(ZGlass* b)
  { fBeta = b; fRayBits |= RB_Beta; }
  void SetBeta(Int_t bid)
  { fBetaId = bid; fRayBits |= RB_BetaId; }
  void SetBeta(const TString& blbl)
  { fBetaLabel = blbl; fRayBits |= RB_BetaLabel; }
  void SetBeta(ZGlass* b, Int_t bid)
  { SetBeta(b); SetBeta(bid); }
  void SetBeta(ZGlass* b, Int_t bid, const TString& blbl)
  { SetBeta(b); SetBeta(bid); SetBeta(blbl); }

  void SetGamma(ZGlass* g)
  { fGamma = g; fRayBits |= RB_Gamma; }
  void SetGamma(Int_t gid)
  { fGammaId = gid; fRayBits |= RB_GammaId; }
  void SetGamma(const TString& glbl)
  { fGammaLabel = glbl; fRayBits |= RB_GammaLabel; }
  void SetGamma(ZGlass* g, Int_t gid)
  { SetGamma(g); SetGamma(gid); }
  void SetGamma(ZGlass* g, Int_t gid, const TString& glbl)
  { SetGamma(g); SetGamma(gid); SetGamma(glbl); }

  //----------------------------------------------------------------------

  void SetRefCnt(Int_t rc);

  TBuffer& CustomBuffer();

  /************************************************************************/
  // Reader side
  /************************************************************************/

  bool HasBeta()         { return fRayBits & RB_Beta; }
  bool HasGamma()        { return fRayBits & RB_Gamma; }
  bool HasCustomBuffer() { return mCustomBuffer != 0; }

  //----------------------------------------------------------------------

  bool IsBasic()       { return fFID.is_basic(); }
  bool IsBasicChange() { return fRQN == RayNS::RQN_change && IsBasic(); }
  bool IsChangeOf(FID_t fid)
  { return fRQN == RayNS::RQN_change && (fFID.is_null() || fFID == fid); }

  //----------------------------------------------------------------------

  void DecRefCnt()
  { mLock.Lock(); if (--mRefCnt == 0) delete this; else mLock.Unlock(); }

  TBuffer& LockCustomBuffer();
  void     UnlockCustomBuffer();

  //----------------------------------------------------------------------

  const char* EventName() const;
  void Dump(ostream& s) const;
};

ostream& operator<<(ostream& s, const Ray& n);


//==============================================================================
// TextMessage
//==============================================================================

struct TextMessage
{
  ZGlass       *fCaller;
  InfoStream_e  fType;
  TString       fMessage;

  TextMessage() {}
  TextMessage(ZGlass* c, InfoStream_e t, const TString& s) :
    fCaller(c), fType(t), fMessage(s) {}
};


//==============================================================================
// EyeCommand
//==============================================================================

struct EyeCommand
{
  enum Command_e { EC_Apocalypse };

  Command_e		fCommand;

  EyeCommand(Command_e c) : fCommand(c) {}
};

#endif

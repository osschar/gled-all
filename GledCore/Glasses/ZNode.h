// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZNode_H
#define GledCore_ZNode_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZList.h>
#include <Stones/ZTrans.h>

class ZNode : public ZList {
  MAC_RNR_FRIENDS(ZNode);

private:
  void		_init();

protected:
  TimeStamp_t	mStampReqTrans;	//! TimeStamp of last change of mTrans

  ZTrans	mTrans;		// X{RPG} Transform from parent

  // Position and Rotation widgets

  // 777 Trans_Pos_Ctrl(Methodbase=>'Pos', Methodname=>'Pos', Transname=>'Trans')

  // 777 Trans_Rot_Ctrl(Methodbase=>'Rot', Methodname=>'Rot',Transname=>'Trans')

  Bool_t	bUseScale;	// X{GS}  7 Bool(-join=>1)
  Bool_t	bUseOM;		// X{GS}  7 Bool(-join=>1)
  Float_t	mOM;		// X{GS}  7 Value(-range=>[-32, 32, 1,1000])
  Float_t	mSx;		// X{GS}  7 Value(-range=>[0,1000, 1,1000], -join=>1)
  Float_t	mSy;		// X{GS}  7 Value(-range=>[0,1000, 1,1000], -join=>1)
  Float_t	mSz;		// X{GS}  7 Value(-range=>[0,1000, 1,1000])

  ZNode*	mParent;	// X{gS} L{l} Structural parent
  Bool_t	bKeepParent;	// X{GS} 7 Bool()

  // RnrBits of RnrMod calculated on the fly.
  Bool_t	bRnrSelf;	// X{GSx} 7 Bool(-join=>1)
  Bool_t	bRnrElements;	// X{GSx} 7 Bool()
public:
  void RnrOnForDaughters();     // X{ED}  7 MButt(-join=>1)
  void RnrOffForDaughters();    // X{ED}  7 MButt()

protected:
  ZGlass*	mRnrMod;	// X{gS} L{}
  Bool_t	bModSelf;	// X{GSx} 7 Bool(-join=>1)
  Bool_t	bModElements;	// X{GSx} 7 Bool()

public:
  ZNode(const Text_t* n="ZNode", const Text_t* t=0) : ZList(n, t) {_init();}

  // ZGlass virtuals
  virtual void SetStamps(TimeStamp_t s)
  { ZList::SetStamps(s); mStampReqTrans = s; }

  // ZList virtuals
  virtual void Add(ZGlass* g);			     // X{E} C{1}
  virtual void AddBefore(ZGlass* g, ZGlass* before); // X{E} C{2}
  virtual void AddFirst(ZGlass* g);		     // X{E} C{1}

  // Overrides for ZTrans stuff that needs stamping
  Int_t	Level();

  Int_t MoveLF(Int_t vi, Float_t amount);		// X{E}
  Int_t Move3(Float_t x, Float_t y, Float_t z);         // X{E}
  Int_t RotateLF(Int_t i1, Int_t i2, Float_t amount);	// X{E}

  Int_t Move(ZNode* ref, Int_t vi, Float_t amount);	// X{E} C{1}
  Int_t Rotate(ZNode* ref, Int_t ii1, Int_t ii2, Float_t amount);// X{E} C{1}
  Int_t SetTrans(const ZTrans& t);	// X{E}
  Int_t MultBy(ZTrans& t);	// X{E}

  Int_t Set3Pos(Float_t x, Float_t y, Float_t z);		  // X{E}
  Int_t SetRotByAngles(Float_t a1, Float_t a2, Float_t a3);  // X{E}
  Int_t SetRotByDegrees(Float_t a1, Float_t a2, Float_t a3); // X{E}

  void SetS(Float_t xx);                           // X{E}
  void SetScales(Float_t x, Float_t y, Float_t z); // X{E}
  void MultS(Float_t s);                           // X{E}

  void SetOMofDaughters(Float_t om, Bool_t enforce_to_all=false); // X{ED}

  ZTrans* ToMFR(int depth=0);
  ZTrans* ToNode(ZNode* top, int depth=0);
  static ZTrans* BtoA(ZNode* a, ZNode* b, ZNode* top=0);

  void FillParentList(list<ZNode*>& plist);
  static ZNode* FindCommonParent(ZNode* a, ZNode* b);
  template <class GLASS>
  GLASS GrepParentByGlass() {
    ZNode* p = mParent;
    if(p == 0) return 0;
    GLASS g = dynamic_cast<GLASS>(p); if(g) return g;
    return p->GrepParentByGlass<GLASS>();
  }

  // Stamps
  void MarkStampReqTrans()
  { mStampReqTrans = ++mTimeStamp; }
  void StampReqTrans()
  { mStampReqTrans = Stamp(FID()); }

  // Clump
  void Spit() const;

#include "ZNode.h7"
  ClassDef(ZNode, 1)
}; // endclass ZNode

GlassIODef(ZNode);

#endif


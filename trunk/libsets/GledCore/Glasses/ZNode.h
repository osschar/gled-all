// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZNode_H
#define GledCore_ZNode_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZList.h>
#include <Stones/ZTrans.h>

class ZNode : public ZList
{
  MAC_RNR_FRIENDS(ZNode);

private:
  void		_init();

protected:
  // ZList
  virtual void on_insert(ZList::iterator it);

  //----------------------------------------------------------------------

  TimeStamp_t	mStampReqTrans;	//! TimeStamp of last change of mTrans
  ZTrans	mTrans;		//  X{RPG} Transform from parent

  // Position and Rotation widgets
  // 777 Trans_Pos_Ctrl(Methodbase=>'Pos', Methodname=>'Pos', Transname=>'Trans')
  // 777 Trans_Rot_Ctrl(Methodbase=>'Rot', Methodname=>'Rot', Transname=>'Trans')

  Bool_t	bUseScale;	// X{GSt}  7 Bool(-join=>1)
  Bool_t	bUseOM;		// X{GSt}  7 Bool(-join=>1)
  Float_t	mOM;		// X{GSt}  7 Value(-range=>[-32, 32, 1,1000])
  Float_t	mSx;		// X{GSt}  7 Value(-range=>[0,1000, 1,1000], -join=>1)
  Float_t	mSy;		// X{GSt}  7 Value(-range=>[0,1000, 1,1000], -join=>1)
  Float_t	mSz;		// X{GSt}  7 Value(-range=>[0,1000, 1,1000])

  ZLink<ZNode>	mParent;	// X{gS} L{} Structural parent
  Bool_t	bKeepParent;	// X{GS} 7 Bool()

  // RnrBits of RnrMod calculated on the fly.
  Bool_t	bRnrSelf;	// X{GSx} 7 Bool(-join=>1)
  Bool_t	bRnrElements;	// X{GSx} 7 Bool()
public:
  void RnrOnForDaughters();     // X{ED}  7 MButt(-join=>1)
  void RnrOffForDaughters();    // X{ED}  7 MButt()
  void RnrElmsOnForDaughters(); // X{ED}  7 MButt(-join=>1)
  void RnrElmsOffForDaughters();// X{ED}  7 MButt()

protected:
  ZLink<ZGlass>	mRnrMod;	// X{gS} L{}
  Bool_t	bModSelf;	// X{GSx} 7 Bool(-join=>1)
  Bool_t	bModElements;	// X{GSx} 7 Bool()
public:
  void MakeRnrModList(ZGlass* optional_element=0); // X{E} C{1} 7 MCWButt()
  void AddRnrMod(ZGlass* el);                      // X{E} C{1}

public:
  ZNode(const Text_t* n="ZNode", const Text_t* t=0) : ZList(n, t) {_init();}

  Int_t	Level();

  // ZTrans wrappers
  //----------------

  void UnitTrans();                // X{E}
  void UnitRot();                  // X{E}

  void SetTrans(const ZTrans& t);  // X{E}
  void MultLeft(const ZTrans& t);  // X{E}
  void MultRight(const ZTrans& t); // X{E}
  ZTrans& ref_trans() { return mTrans; } // Use wisely.

  void MoveLF(Int_t vi, Double_t amount);             // X{E}
  void Move3LF(Double_t x, Double_t y, Double_t z);   // X{E}
  void RotateLF(Int_t i1, Int_t i2, Double_t amount); // X{E}

  void MovePF(Int_t vi, Double_t amount);             // X{E}
  void Move3PF(Double_t x, Double_t y, Double_t z);   // X{E}
  void RotatePF(Int_t i1, Int_t i2, Double_t amount); // X{E}

  void Move(ZNode* ref, Int_t vi, Double_t amount);               // X{E} C{1}
  void Move3(ZNode* ref, Double_t x, Double_t y, Double_t z);     // X{E} C{1}
  void Rotate(ZNode* ref, Int_t ii1, Int_t ii2, Double_t amount); // X{E} C{1}

  void SetPos(Double_t x, Double_t y, Double_t z);          // X{E}
  void SetRotByAngles(Float_t a1, Float_t a2, Float_t a3);  // X{E}
  void SetRotByDegrees(Float_t a1, Float_t a2, Float_t a3); // X{E}
  void SetRotByAnyAngles(Float_t a1, Float_t a2, Float_t a3, const Text_t* pat);  // X{E}
  void SetRotByAnyDegrees(Float_t a1, Float_t a2, Float_t a3, const Text_t* pat); // X{E}

  // Scaling (stored separately from ZTrans, use ApplyScale to imprint it)

  void SetScale(Float_t xx);                       // X{E}
  void SetScales(Float_t x, Float_t y, Float_t z); // X{E}
  void MultScale(Float_t s);                       // X{E}
  void ApplyScale(ZTrans& t);

  void SetOMofDaughters(Float_t om, Bool_t enforce_to_all=false); // X{ED}

  // Node-to-node transforamtions

  ZTrans* ToMFR(int depth=0);
  ZTrans* ToNode(ZNode* top, int depth=0);
  static ZTrans* BtoA(ZNode* a, ZNode* b, ZNode* top=0);

  void FillParentList(list<ZNode*>& plist);
  static ZNode* FindCommonParent(ZNode* a, ZNode* b);
  template <class GLASS>
  GLASS* GrepParentByGlass() {
    ZNode* p = *mParent;
    if(p == 0) return 0;
    GLASS* g = dynamic_cast<GLASS*>(p); if(g) return g;
    return p->GrepParentByGlass<GLASS>();
  }

  // ZGlass virtuals
  virtual void SetStamps(TimeStamp_t s)
  { ZList::SetStamps(s); mStampReqTrans = s; }

  // Stamps
  void MarkStampReqTrans() { mStampReqTrans = ++mTimeStamp; }
  void StampReqTrans()     { mStampReqTrans = Stamp(FID()); }

#include "ZNode.h7"
  ClassDef(ZNode, 1);
}; // endclass ZNode

#endif

// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZNode_H
#define Gled_ZNode_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZList.h>
#include <Stones/ZTrans.h>

#include <list>

class ZNode : public ZList {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(ZNode);

private:
  void		_init();

protected:
  TimeStamp_t	mStampReqTrans;	//! TimeStamp of last change of mTrans

  ZTrans	mTrans;		// X{RP} Transform from parent

  // Position and Rotation widgets

  // 777 Trans_Pos_Ctrl(Methodbase=>'Pos', Transname=>'Trans')

  // 777 Trans_Rot_Ctrl(Methodbase=>'Rot', Transname=>'Trans')

  ZNode*	mParent;	// X{gS} L{l} Structural parent

  Bool_t	bKeepParent;	// X{GS} 7 Bool()

public:
  // Constructors
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
  Int_t MoveLF(Int_t vi, Real_t amount);		// X{E}
  Int_t RotateLF(Int_t i1, Int_t i2, Real_t amount);	// X{E}
  Int_t Move(ZNode* ref, Int_t vi, Real_t amount);	// X{E} C{1}
  Int_t Rotate(ZNode* ref, Int_t ii1, Int_t ii2, Real_t amount);// X{E} C{1}
  Int_t SetTrans(ZTrans& t);	// X{E}
  Int_t MultBy(ZTrans& t);	// X{E}

  Int_t Set3Pos(Real_t x, Real_t y, Real_t z);		// X{E}
  Int_t SetRotByAngles(Real_t a1, Real_t a2, Real_t a3); // X{E}
  Int_t SetRotByDegrees(Real_t a1, Real_t a2, Real_t a3); // X{E}

  ZTrans* ToMFR();
  ZTrans* ToNode(ZNode* top);
  static ZTrans* BtoA(ZNode* a, ZNode* b);

  // Clickotations
  void Spit() const;	// X{E}

#include "ZNode.h7"
  ClassDef(ZNode, 1)
}; // endclass ZNode

GlassIODef(ZNode);

typedef list<ZNode*>		lpZNode_t;
typedef list<ZNode*>::iterator	lpZNode_i;

#endif


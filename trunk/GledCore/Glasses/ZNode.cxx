// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNode.h"
#include "ZNode.c7"
#include "ZQueen.h"
#include "ZRnrModList.h"
#include <Stones/ZComet.h>

#include <TMath.h>

#include <ctype.h>

typedef list<ZNode*>           lpZNode_t;
typedef list<ZNode*>::iterator lpZNode_i;

ClassImp(ZNode);

namespace {
  // Limit max search depth for
  int ZNodeTransSearchMaxDepth = 100;
}

void ZNode::_init()
{
  bUseScale = false;
  bUseOM    = false;
  mOM = 0;
  mSx = mSy = mSz = 1;
  mParent = 0; bKeepParent = true;
  mRnrMod = 0;
  bRnrSelf = bRnrElements = bModSelf = bModElements = true;

  // SetElementFID(ZNode::FID()); // Forces ZNode children only
}

/**************************************************************************/
// ZList virtuals
/**************************************************************************/

void ZNode::on_insert(ZList::iterator it)
{
  // Set parent if possible; honour policy bKeepParent policy of the
  // new node.

  ZNode* n = dynamic_cast<ZNode*>(it.lens());
  if(n == 0 || n == this)
    return;
  if((n->mParent == 0 || n->bKeepParent == false) &&
     (n->mQueen->DependsOn(mQueen)))
    {
      n->SetParent(this);
    }
}

/**************************************************************************/

Int_t ZNode::Level()
{
  Int_t l = 0;
  ZNode *p = this;
  while((p = p->GetParent())) ++l;
  return l;
}

/**************************************************************************/
// ZTrans wrappers
/**************************************************************************/

void ZNode::SetTrans(const ZTrans& t)
{
  mTrans.SetTrans(t);
  mStampReqTrans = Stamp(FID());
}

void ZNode::MultLeft(const ZTrans& t)
{
  mTrans.MultLeft(t);
  mStampReqTrans = Stamp(FID());
}

void ZNode::MultRight(const ZTrans& t)
{
  mTrans.MultRight(t);
  mStampReqTrans = Stamp(FID());
}

/**************************************************************************/

void ZNode::MoveLF(Int_t vi, Double_t amount)
{
  if(vi<1 || vi>3) return;
  mTrans.MoveLF(vi, amount);
  mStampReqTrans = Stamp(FID());
}

void ZNode::Move3LF(Double_t x, Double_t y, Double_t z)
{
  mTrans.Move3LF(x,y,z);
  mStampReqTrans = Stamp(FID());
}

void ZNode::RotateLF(Int_t i1, Int_t i2, Double_t amount)
{
  if(i1<1 || i1>3 || i2<1 || i2>3) return;
  mTrans.RotateLF(i1, i2, amount);
  mStampReqTrans = Stamp(FID());
}

/**************************************************************************/

void ZNode::MovePF(Int_t vi, Double_t amount)
{
  if(vi<1 || vi>3) return;
  mTrans.MovePF(vi, amount);
  mStampReqTrans = Stamp(FID());
}

void ZNode::Move3PF(Double_t x, Double_t y, Double_t z)
{
  mTrans.Move3PF(x,y,z);
  mStampReqTrans = Stamp(FID());
}

void ZNode::RotatePF(Int_t i1, Int_t i2, Double_t amount)
{
  if(i1<1 || i1>3 || i2<1 || i2>3) return;
  mTrans.RotatePF(i1, i2, amount);
  mStampReqTrans = Stamp(FID());
}

/**************************************************************************/

void ZNode::Move(ZNode* ref, Int_t vi, Double_t amount)
{
  static const Exc_t _eh("ZNode::Move ");

  if(ref == 0) throw _eh + "called with zero reference.";
  if(vi<1 || vi>3) return;

  if(ref == this) {
    mTrans.MoveLF(vi, amount);
  } else {
    auto_ptr<ZTrans> a( BtoA(*mParent, ref) );
    throw _eh + "no path from " + Identify() + " to reference node " + ref->Identify() + ".";
    mTrans.Move(*a, vi, amount);
  }
  mStampReqTrans = Stamp(FID());
}

void ZNode::Move3(ZNode* ref, Double_t x, Double_t y, Double_t z)
{
  static const Exc_t _eh("ZNode::Move3 ");

  if(ref == 0) throw _eh + "called with zero reference.";

  if(ref == this) {
    mTrans.Move3LF(x, y, z);
  } else {
    auto_ptr<ZTrans> a( BtoA(*mParent, ref) );
    throw _eh + "no path from " + Identify() + " to reference node " + ref->Identify() + ".";
    mTrans.Move3(*a, x, y, z);
  }
  mStampReqTrans = Stamp(FID());
}

void ZNode::Rotate(ZNode* ref, Int_t ii1, Int_t ii2, Double_t amount)
{
  static const Exc_t _eh("ZNode::Rotate ");

  if(ref == 0) throw _eh + "called with zero reference.";
  if(ii1<1 || ii1>3 || ii2<1 || ii1>3) return;
  if(ref == this) {
    // Get segv from m^-1 * m
    mTrans.RotateLF(ii1, ii2, amount);
  } else {
    auto_ptr<ZTrans> a( BtoA(*mParent, ref) );
    if(a.get() == 0)
      throw _eh + "no path from " + Identify() + " to reference node " + ref->Identify() + ".";
    mTrans.Rotate(*a, ii1, ii2, amount);
  }
  mStampReqTrans = Stamp(FID());
}


/**************************************************************************/
// All possible Set methods.
/**************************************************************************/

void ZNode::SetPos(Double_t x, Double_t y, Double_t z)
{
  mTrans.SetPos(x,y,z); mStampReqTrans = Stamp(FID());
}

void ZNode::SetRotByAngles(Float_t a1, Float_t a2, Float_t a3)
{
  mTrans.SetRotByAngles(a1, a2, a3);
  mStampReqTrans = Stamp(FID());
}

void ZNode::SetRotByDegrees(Float_t a1, Float_t a2, Float_t a3)
{
  const Float_t f = TMath::DegToRad();
  SetRotByAngles(f*a1, f*a2, f*a3);
}

void ZNode::SetRotByAnyAngles(Float_t a1, Float_t a2, Float_t a3, const Text_t* pat)
{
  mTrans.SetRotByAnyAngles(a1, a2, a3, pat);
  mStampReqTrans = Stamp(FID());
}

void ZNode::SetRotByAnyDegrees(Float_t a1, Float_t a2, Float_t a3, const Text_t* pat)
{
  const Float_t f = TMath::DegToRad();
  SetRotByAnyAngles(f*a1, f*a2, f*a3, pat);
}

/**************************************************************************/

void ZNode::SetScale(Float_t s)
{
  mSx = mSy = mSz = s;
  mStampReqTrans = Stamp(FID());
}

void ZNode::SetScales(Float_t x, Float_t y, Float_t z)
{
  mSx=x; mSy=y; mSz=z;
  mStampReqTrans = Stamp(FID());
}

void ZNode::MultScale(Float_t s)
{
  mSx*=s; mSy*=s; mSz*=s;
  mStampReqTrans = Stamp(FID());
}

void ZNode::ApplyScale(ZTrans& t)
{
  if(bUseScale) {
    t.Scale(mSx, mSy, mSz);
  }
  if(bUseOM && mParent != 0 && mParent->bUseOM) {
    const Float_t dom =  mOM - mParent->mOM;
    if(dom != 0) {
      const Double_t s = TMath::Power(10, dom);
      t.Scale(s, s, s);
    }
  }
}

/**************************************************************************/

// Methods for changing properties of daugters.

void ZNode::RnrOnForDaughters()
{
  lpZNode_t dts; CopyListByGlass<ZNode>(dts);
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    GLensReadHolder _rlck(*i);
    (*i)->SetRnrSelf(true);
  }
}

void ZNode::RnrOffForDaughters()
{
  lpZNode_t dts; CopyListByGlass<ZNode>(dts);
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    GLensReadHolder _rlck(*i);
    (*i)->SetRnrSelf(false);
  }
}

void ZNode::RnrElmsOnForDaughters()
{
  lpZNode_t dts; CopyListByGlass<ZNode>(dts);
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    GLensReadHolder _rlck(*i);
    (*i)->SetRnrElements(true);
  }
}

void ZNode::RnrElmsOffForDaughters()
{
  lpZNode_t dts; CopyListByGlass<ZNode>(dts);
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    GLensReadHolder _rlck(*i);
    (*i)->SetRnrElements(false);
  }
}

void ZNode::MakeRnrModList(ZGlass* optional_element)
{
  // Makes sure that link mRnrMod is pointing to a ZRnrModList.
  // Previous contents of the link is added to the list if it is
  // created.
  // If non-zero, 'optional_element' is also added to the list.

  ZRnrModList* rml = dynamic_cast<ZRnrModList*>(mRnrMod.get());
  if(rml == 0) {
    rml = new ZRnrModList;
    mQueen->CheckIn(rml);
    if(mRnrMod != 0)
      rml->Add(mRnrMod.get());
    SetRnrMod(rml);
  }
  if(optional_element)
    rml->Add(optional_element);
}

void ZNode::SetOMofDaughters(Float_t om, Bool_t enforce_to_all)
{
  // If enforce_to_all also changes OM of children w/ parent != this.

  lpZNode_t dts; CopyListByGlass<ZNode>(dts);
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    ZNode* d = *i;
    GLensReadHolder _rlck(d);
    if(enforce_to_all || d->mParent == this) {
      d->SetOM(om);
    }
  }
}

/**************************************************************************/

ZTrans* ZNode::ToMFR(int depth)
{
  if(depth > ZNodeTransSearchMaxDepth) {
    ISerr("ZNode::ToMFR max search depth exceeded.");
    return 0;
  }

  ZNode *p = mParent.get();
  ZTrans* x;
  if(p == 0) {
    ReadLock();
    x = new ZTrans(mTrans);
    if(bUseScale) { x->Scale(mSx, mSy, mSz); }
    ReadUnlock();
  } else {
    x = p->ToMFR(++depth);
    if(x == 0) return 0;
    ReadLock();
    *x *= mTrans;
    if(bUseScale) { x->Scale(mSx, mSy, mSz); }
    if(bUseOM && p->bUseOM) {
      Float_t dom =  mOM - p->mOM;
      if(dom != 0) {
	const Double_t s = TMath::Power(10, dom);
	x->Scale(s, s, s);
      }
    }
    ReadUnlock();
  }
  return x;
}

ZTrans* ZNode::ToNode(ZNode* top, int depth)
{
  if(depth > ZNodeTransSearchMaxDepth)  {
    ISerr("ZNode::ToNode max search depth exceeded.");
    return 0;
  }

  ZNode *p = *mParent;
  if(p == 0) return 0;

  ZTrans* x;
  if(p == top) {
    GLensReadHolder rd_lck(this);
    x = new ZTrans(mTrans);
  } else {
    x = p->ToNode(top, ++depth);
    if(x) {
      GLensReadHolder rd_lck(this);
      *x *= mTrans;
    } else {
      return 0;
    }
  }

  if(bUseScale) { x->Scale(mSx, mSy, mSz); }
  if(bUseOM && p->bUseOM) {
    Float_t dom =  mOM - p->mOM;
    if(dom != 0) {
      const Double_t s = TMath::Power(10, dom);
      x->Scale(s, s, s);
    }
  }

  return x;
}

/**************************************************************************/

ZTrans* ZNode::BtoA(ZNode* a, ZNode* b, ZNode* top)
{
  if(top == 0) {
    top = FindCommonParent(a, b);
    if(top == 0) return 0;
  }
  auto_ptr<ZTrans> at (a->ToNode(top));
  if(at.get() == 0) return 0;
  at->Invert();
  auto_ptr<ZTrans> bt (b->ToNode(top));
  if(bt.get() == 0) return 0;
  *at *= *bt;
  return at.release();
}

/**************************************************************************/

void ZNode::FillParentList(list<ZNode*>& plist)
{
  ZNode* p = *mParent;
  if(p != 0) {
    plist.push_back(p);
    p->FillParentList(plist);
  }
}

ZNode* ZNode::FindCommonParent(ZNode* a, ZNode* b)
{
  if(a == 0 || b == 0) return 0;
  if(a == b) return a;

  // !! This far from optimal, methinks.

  lpZNode_t l[2];
  l[0].push_back(a); a->FillParentList(l[0]);
  l[1].push_back(b); b->FillParentList(l[1]);
  int x = 0, y = 1;
  do {
    lpZNode_i i = find(l[y].begin(), l[y].end(), l[x].front());
    if(i != l[y].end()) return l[x].front();
    l[x].pop_front();
    y = x; x = 1 - x;
  } while(!l[0].empty() && !l[1].empty());
  return 0;
}

// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNode.h"
#include <Stones/ZMIR.h>
#include <Stones/ZComet.h>
#include <Ephra/Saturn.h>
#include <Stones/ZComet.h>

#include <ctype.h>

ClassImp(ZNode)

namespace {
  // Limit max search depth for 
  int ZNodeTransSearchMaxDepth = 100;
}

void ZNode::_init()
{
  bUseScale = false;
  mSx = mSy = mSz = 1;
  mParent = 0; bKeepParent = true;

  // SetElementFID(ZNode::FID()); // Forces ZNode children only
}

/**************************************************************************/
// ZList virtuals
/**************************************************************************/

// Method SetParent is auto generated.
// Perhaps want manual version that will do list remove. ????
// On the other hand it could also be done here.
// Or require all cals to Add to also remove it from previous list.
// Or provide AdMigration virtual base in ZGlass ... hmmmph

// Locks done in ZList, SetParent

// Yoohoo ... here are some real problems with adding nodes to nodes in lower
// kings. Need TakeParentship method!
// Beta should be relatively permissive ... to allow local collections
// and links to global objects.

void ZNode::Add(ZGlass* g)
{
  ZNode* n = dynamic_cast<ZNode*>(g);
  if(n==this) return;
  if(n && !n->bKeepParent && n->mParent)
    n->mParent->Remove(g);
  ZList::Add(g);
  if(n && (n->mParent == 0 || (n->mParent !=0 && !n->bKeepParent)))
    n->SetParent(this);
}

void ZNode::AddBefore(ZGlass* g, ZGlass* before)
{
  ZNode* n = dynamic_cast<ZNode*>(g);
  if(n==this) return;
  if(n && !n->bKeepParent && n->mParent)
    n->mParent->Remove(g);
  ZList::AddBefore(g, before);
  if(n && (n->mParent == 0 || (n->mParent !=0 && !n->bKeepParent)))
    n->SetParent(this);
}

void ZNode::AddFirst(ZGlass* g)
{
  ZNode* n = dynamic_cast<ZNode*>(g);
  if(n==this) return;
  if(n && !n->bKeepParent && n->mParent)
    n->mParent->Remove(g);
  ZList::AddFirst(g);
  if(n && (n->mParent == 0 || (n->mParent !=0 && !n->bKeepParent)))
    n->SetParent(this);
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
// Wrappers for ZTrans
/**************************************************************************/

Int_t ZNode::MoveLF(int vi, Float_t amount)
{
  if(vi<0 || vi>3) return 0;
  mTrans.MoveLF((Int_t)vi, amount);
  mStampReqTrans = Stamp(LibID(), ClassID());
  return 1;
}

Int_t ZNode::RotateLF(Int_t i1, Int_t i2, Float_t amount)
{
  if(i1<0 || i1>3 || i2<0 || i2>3) return 0;
  mTrans.RotateLF((Int_t)i1, (Int_t)i2, amount);
  mStampReqTrans = Stamp(LibID(), ClassID());
  return 1;
}

Int_t ZNode::Move(ZNode* ref, Int_t vi, Float_t amount)
{
  if(ref==0) return MoveLF(vi, amount);
  if(vi<0 || vi>3) return 0;
  if(ref == this) {
    mTrans.MoveLF((Int_t)vi, amount);
  } else {
    ZTrans* a = BtoA(mParent, ref);
    mTrans.Move(a, (Int_t)vi, amount);
    delete a;
  }
  mStampReqTrans = Stamp(LibID(), ClassID());
  return 1;
}

Int_t ZNode::Rotate(ZNode* ref, Int_t ii1, Int_t ii2, Float_t amount)
{
  if(ref==0) return RotateLF(ii1, ii2, amount);
  if(ii1<0 || ii1>3 || ii2<0 || ii1>3) return 0;
  if(ref == this) {
    // Get explosion from m^-1 * m
    mTrans.RotateLF((Int_t)ii1, (Int_t)ii2, amount);
  } else {
    ZTrans* a = BtoA(mParent, ref);
    mTrans.Rotate(a, (Int_t)ii1, (Int_t)ii2, amount);
    delete a;
  }

  mStampReqTrans = Stamp(LibID(), ClassID());
  return 1;
}

Int_t ZNode::SetTrans(ZTrans& t)
{
  mTrans.SetTrans(t);
  mStampReqTrans = Stamp(LibID(), ClassID());
  return 0;
}

Int_t ZNode::MultBy(ZTrans& t)
{
  mTrans *= t;
  mStampReqTrans = Stamp(LibID(), ClassID());
  return 0;
}

/**************************************************************************/

Int_t ZNode::Set3Pos(Float_t x, Float_t y, Float_t z)
{
  mTrans.Set3Pos(x,y,z); mStampReqTrans = Stamp(LibID(), ClassID()); return 1;
}

Int_t ZNode::SetRotByAngles(Float_t a1, Float_t a2, Float_t a3)
{
  mTrans.SetRotByAngles(a1,a2,a3);
  mStampReqTrans = Stamp(LibID(), ClassID());
  return 1;
}

Int_t ZNode::SetRotByDegrees(Float_t a1, Float_t a2, Float_t a3)
{
  Float_t f = TMath::Pi()/180;
  SetRotByAngles(f*a1, f*a2, f*a3);
  return 1;
}

/**************************************************************************/

void ZNode::SetS(Float_t xx)
{
  WriteLock();
  mSx = mSy = mSz = xx;
  mStampReqTrans = Stamp(LibID(), ClassID());
  WriteUnlock();
}

void ZNode::SetScales(Float_t x, Float_t y, Float_t z)
{
  WriteLock();
  mSx=x; mSy=y; mSz=z;
  mStampReqTrans = Stamp(LibID(), ClassID());
  WriteUnlock();
}

void ZNode::MultS(Float_t s)
{
  WriteLock();
  mSx*=s; mSy*=s; mSz*=s;
  mStampReqTrans = Stamp(LibID(), ClassID());
  WriteUnlock();
}

/**************************************************************************/

ZTrans* ZNode::ToMFR(int depth)
{
  if(depth > ZNodeTransSearchMaxDepth) {
    ISerr("ZNode::ToMFR max search depth exceeded.");
    return 0;
  }

  ZNode *p = mParent;
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

  ZNode *p = mParent;
  if(p == 0) return 0;

  ZTrans* x;
  if(p == top) {
    ReadLock();
    x = new ZTrans(mTrans);
    if(bUseScale) { x->Scale(mSx, mSy, mSz); }
    ReadUnlock();
  } else {
    x = p->ToNode(top, ++depth);
    if(x) {
      ReadLock();
      *x *= mTrans;
      if(bUseScale) { x->Scale(mSx, mSy, mSz); }
      ReadUnlock();
    }
  }
  return x;
}

/**************************************************************************/

ZTrans* ZNode::BtoA(ZNode* a, ZNode* b)
{
  ZTrans* at = a->ToMFR(); if(at == 0) { return 0; }
  at->Invert();
  ZTrans* bt = b->ToMFR(); if(bt == 0) { delete at; return 0; }
  *at *= *bt;
  delete bt;
  return at;
}

/**************************************************************************/
/*** Stupid junk ***/
/**************************************************************************/

void ZNode::Spit() const
{
  cout << GetName() << endl << mTrans;
}

/**************************************************************************/

#include <TBuffer.h>

// Example of NodeLists() method
// lpZList_t*
// ZSth::NodeLists() {
//   // Returns list of ZList pointers ... needed by ZBush
//   // Assume we have a member ZList mTubes ... than you should:
//   l = ZNode::NodeLists();
//   l->push_back(&mTubes);
//   return l;
// }

/**************************************************************************/

ostream& operator<<(ostream& s, const ZNode& n) {
  const ZTrans& t = n.RefTrans();
  for(Int_t i=0; i<=3; i++) {
    s << t(i,4u) << "\t|\t";
    for(Int_t j=0; j<=3; j++)
      s << t(i,j) << ((j==3) ? "\n" : "\t");
  }
  return s;
}

#include "ZNode.c7"

// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// RnrDriver
//
// Generic RnrDriver implementation.
// Currently still GL-only, as infrastructure for POV-Ray rendering is not
// yet prepared.

#include "RnrDriver.h"
#include <Eye/Eye.h>
#include <Glasses/ZGlass.h>
#include <Glasses/ZNode.h>

namespace OS   = OptoStructs;
namespace GVNS = GledViewNS;

/**************************************************************************/

RnrDriver::RnrDriver(Eye* e, const string& r) : mEye(e), mRnrName(r)
{
  bUseOwnRnrs = false;

  mRnrCount = 0;
  bDryRun   = false;

  mMaxDepth = 100;

  mMaxLamps = 8;
  mLamps = new (A_Rnr*)[mMaxLamps];

  mMaxClipPlanes = 6;
  mClipPlanes = new (A_Rnr*)[mMaxClipPlanes];

  bMarkupNodes = false;

  mEye->RegisterImageConsumer(this);
}

RnrDriver::~RnrDriver()
{
  mEye->UnregisterImageConsumer(this);
  delete [] mLamps;
}

/**************************************************************************/

void RnrDriver::ImageDeath(OS::ZGlassImg* img)
{
  hImg2Rnr_i oi = mOwnRnrs.find(img);
  if(oi != mOwnRnrs.end()) {
    delete oi->second;
    mOwnRnrs.erase(oi);
  }
}

/**************************************************************************/

void RnrDriver::FillRnrScheme(RnrScheme* rs, A_Rnr* rnr,
		   const GledViewNS::RnrBits& bits)
{
  vlRnrElement_t& rev = rs->fScheme;
  if(bits.fSelf[0]) {
    rev[bits.fSelf[0]].push_back(RnrElement(rnr, &A_Rnr::PreDraw));
  }
  if(bits.fSelf[1]) {
    rev[bits.fSelf[1]].push_back(RnrElement(rnr, &A_Rnr::Draw));
  }
  if(bits.fSelf[2]) {
    rev[bits.fSelf[2]].push_back(RnrElement(rnr, &A_Rnr::PostDraw));
  }
  if(bits.fSelf[3]) {
    rev[bits.fSelf[3]].push_back(RnrElement(rnr, 0));
  }
}

void RnrDriver::FillRnrScheme(RnrScheme* rs, OS::lpZGlassImg_t* imgs,
		   const GledViewNS::RnrBits& bits)
{
  vlRnrElement_t& rev = rs->fScheme;
  if(bits.fList[0]) {
    for(OS::lpZGlassImg_i img=imgs->begin(); img!=imgs->end(); ++img)
      rev[bits.fList[0]].push_back(RnrElement(GetRnr(*img), &A_Rnr::PreDraw));
  }
  if(bits.fList[1]) {
    for(OS::lpZGlassImg_i img=imgs->begin(); img!=imgs->end(); ++img)
      rev[bits.fList[1]].push_back(RnrElement(GetRnr(*img), &A_Rnr::Draw));
  }
  if(bits.fList[2]) {
    for(OS::lpZGlassImg_i img=imgs->begin(); img!=imgs->end(); ++img)
      rev[bits.fList[2]].push_back(RnrElement(GetRnr(*img), &A_Rnr::PostDraw));
  }
  if(bits.fList[3]) {
    for(OS::lpZGlassImg_i img=imgs->begin(); img!=imgs->end(); ++img)
      rev[bits.fList[3]].push_back(RnrElement(GetRnr(*img), 0));
  }
}

/**************************************************************************/
/**************************************************************************/

A_Rnr* RnrDriver::GetLensRnr(ZGlass* lens)
{
  // This one could be extended to use internal/stack-dependant
  // resolution and thus support multiple renderers.

  OS::ZGlassImg* img = mEye->DemanglePtr(lens);
  if(img) return AssertDefRnr(img);
  return 0;
}

A_Rnr* RnrDriver::AssertDefRnr(OS::ZGlassImg* img)
{
  // This one is somewhat tricky: A_Rnr is A_View, but constructed
  // with 0 image. Thus we set it after.

  if(img->fDefRnr != 0) return img->fDefRnr;

  hImg2Rnr_i oi = mOwnRnrs.find(img);
  if(oi != mOwnRnrs.end()) return oi->second;

  A_Rnr* rnr = img->fClassInfo->fViewPart->SpawnRnr(mRnrName, img->fGlass);
  assert(rnr != 0);
  rnr->SetImg(img);

  if(rnr->bOnePerRnrDriver)
    mOwnRnrs[img] = rnr;
  else
    img->fDefRnr = rnr;

  return rnr;
}

void RnrDriver::Render(A_Rnr* rnr)
{
  static const string _eh("RnrDriver::Render ");

  if(mMaxDepth <= 0) return;
  --mMaxDepth;

  if(rnr->mRnrScheme == 0) {
    rnr->mRnrScheme = new RnrScheme;
    rnr->fImg->fGlass->ReadLock();
    rnr->CreateRnrScheme(this);
    rnr->fImg->fGlass->ReadUnlock();
  }

  for(UChar_t rl=1; rl<=A_Rnr::sMaxRnrLevel; ++rl) {
    lRnrElement_t& re_list = rnr->mRnrScheme->fScheme[rl];
    for(lRnrElement_i re=re_list.begin(); re!=re_list.end(); ++re) {
      if(re->fRnrFoo == 0) {
	// printf("%sDescending to %s, from %s, rl=%d\n", _eh.c_str(), re->fRnr->fImg->fGlass->Identify().c_str(), rnr->fImg->fGlass->Identify().c_str(), rl);
	Render(re->fRnr);
      } else {
	re->fRnr->fImg->fGlass->ReadLock();
	((re->fRnr)->*(re->fRnrFoo))(this);
	re->fRnr->fImg->fGlass->ReadUnlock();
      }
    }
  }

  ++mMaxDepth;
}

/**************************************************************************/
/**************************************************************************/

void RnrDriver::BeginRender()
{
  ++mRnrCount;
  mPMStack.push_back(PMSEntry());
  for(int i=0; i<mMaxLamps; ++i) {
    mLamps[i] = 0;
  } 
  for(int i=0; i<mMaxClipPlanes; ++i) {
    mClipPlanes[i] = 0;
  } 
}


void RnrDriver::EndRender()
{
  for(int l=0; l<mMaxLamps; ++l) {
    if(mLamps[l] != 0) {
      // cout <<"RnrDriver cleaning-up a dirty lamp ...\n";
      mLamps[l]->CleanUp(this);
    }
  }
  for(int l=0; l<mMaxClipPlanes; ++l) {
    if(mClipPlanes[l] != 0) {
      cout <<"RnrDriver cleaning-up a dirty clip-plane ...\n";
      mClipPlanes[l]->CleanUp(this);
    }
  }
  mPMStack.pop_back();
}

/**************************************************************************/
// RnrDriver::PMSEntry
/**************************************************************************/

RnrDriver::PMSEntry::PMSEntry(PMSEntry* p, ZNode* n) :
      fPrev(p), fNode(n), bTo(0), bFrom(0), fToGCS(n)
{
  n->ApplyScale(fToGCS);
}

ZTrans& RnrDriver::PMSEntry::ToGCS()
{
  if(bTo == false && fPrev != 0) {
    fToGCS = fPrev->ToGCS()*fToGCS;
    bTo = true;
  }
  return fToGCS;
}

ZTrans& RnrDriver::PMSEntry::FromGCS()
{
  if(bFrom == false) {
    fFromGCS = ToGCS();
    fFromGCS.InvertFast();
    bFrom = true;
  }
  return fFromGCS;
} 

/**************************************************************************/
/**************************************************************************/

Int_t RnrDriver::GetLamp(A_Rnr* rnr)
{
  for(Int_t i=0; i<mMaxLamps; ++i) {
    if(mLamps[i] == 0) { mLamps[i] = rnr; return i; }
  }
  return -1;
}

void RnrDriver::ReturnLamp(Int_t lamp)
{
  mLamps[lamp] = 0;
}

/**************************************************************************/

Int_t RnrDriver::GetClipPlane(A_Rnr* rnr)
{
  for(Int_t i=0; i<mMaxClipPlanes; ++i) {
    if(mClipPlanes[i] == 0) { mClipPlanes[i] = rnr; return i; }
  }
  return -1;
}

void RnrDriver::ReturnClipPlane(Int_t clip)
{
  mClipPlanes[clip] = 0;
}


/**************************************************************************/
// RnrMods interface
/**************************************************************************/

void RnrDriver::SetDefRnrMod(FID_t fid, RnrMod* ud)
{
  if(find_rnrmod(fid)) {
    mRMI->second.def = ud;
  } else {
    mRMStacks[fid].def = ud;
  }
}

RnrMod* RnrDriver::GetDefRnrMod(FID_t fid)
{
  if(find_rnrmod(fid)) {
    return mRMI->second.def;
  } else {
    return 0;
  }
}

void RnrDriver::PushRnrMod(FID_t fid, RnrMod* ud)
{
  if(find_rnrmod(fid)) {
    mRMI->second.stack.push(ud);
  } else {
    mRMStacks[fid].stack.push(ud);
  }
}

RnrMod* RnrDriver::PopRnrMod(FID_t fid)
{
  if(find_rnrmod(fid) && ! mRMI->second.stack.empty()) {
    RnrMod* r = mRMI->second.stack.top();
    mRMI->second.stack.pop();
    return r;
  } else {
    return 0;
  }
}

RnrMod* RnrDriver::TopRnrMod(FID_t fid)
{
  if(find_rnrmod(fid) && ! mRMI->second.stack.empty()) {
    return mRMI->second.stack.top();
  } else {
    return 0;
  }
}

RnrMod* RnrDriver::GetRnrMod(FID_t fid)
{
  if(find_rnrmod(fid) == false) {
    mRMI = mRMStacks.insert(make_pair(fid, RMStack())).first;
    mRMFid = fid;
  }

  if(mRMI->second.stack.empty()) {
    if(mRMI->second.def) return mRMI->second.def;
    if(mRMI->second.def_autogen == 0) {
      ZGlass* lens = GledNS::ConstructLens(fid);
      A_Rnr*  rnr  = lens->VGlassInfo()->fViewPart->SpawnRnr(mRnrName, lens);
      mRMI->second.def_autogen = new RnrMod(lens, rnr);
    }
    return mRMI->second.def_autogen;
  } else {
    return mRMI->second.stack.top();
  }
}

/**************************************************************************/

void RnrDriver::RemoveRnrModEntry(FID_t fid)
{
  if(find_rnrmod(fid)) {
    if(mRMI->second.def_autogen != 0) {
      RnrMod* rm = mRMI->second.def_autogen;
      delete rm->fRnr;
      delete rm->fLens;
      delete rm;
    }
    mRMStacks.erase(mRMI); mRMFid.clear();
  }
}

void RnrDriver::CleanUpRnrModDefaults()
{
  static const string _eh("RnrDriver::CleanUpRnrModDefaults ");

  for(hRMStack_i i=mRMStacks.begin(); i!=mRMStacks.end(); ++i) {
    i->second.def = 0;
    int cnt = 0; 
    while( ! i->second.stack.empty()) {
      ++cnt;
      i->second.stack.pop();
    }
    if(cnt)
      printf("%sstack for '%s' not empty (%d entries remained).",
	     _eh.c_str(), GledNS::FindClassInfo(i->first)->fName.c_str(), cnt);
  }
}

/**************************************************************************/

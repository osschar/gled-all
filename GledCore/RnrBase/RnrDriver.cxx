// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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

namespace OS   = OptoStructs;
namespace GVNS = GledViewNS;

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

  if(img->fDefRnr == 0) {
    img->fDefRnr = img->fClassInfo->fViewPart->SpawnRnr(mRnrName, img->fGlass);
    if(img->fDefRnr) {
      img->fDefRnr->SetImg(img);
    }
  }
  return img->fDefRnr;
}

void RnrDriver::Render(A_Rnr* rnr)
{
  static const string _eh("RnrDriver::Render ");

  if(mMaxDepth <= 0) return;
  --mMaxDepth;

  if(rnr->mRnrScheme == 0) {
    rnr->mRnrScheme = new RnrScheme;
    rnr->CreateRnrScheme(this);
  }

  for(UChar_t rl=1; rl<=A_Rnr::sMaxRnrLevel; ++rl) {
    lRnrElement_t& re_list = rnr->mRnrScheme->fScheme[rl];
    for(lRnrElement_i re=re_list.begin(); re!=re_list.end(); ++re) {
      if(re->fRnrFoo == 0) {
	// cout << _eh + "Descending to " << re->fRnr->fImg->fGlass->GetName() <<endl;
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
/**************************************************************************/

void RnrDriver::InitLamps()
{
  for(int i=0; i<mMaxLamps; ++i) {
    mLamps[i] = 0;
  }
}

int RnrDriver::GetLamp(A_Rnr* l_rnr)
{
  for(int i=0; i<mMaxLamps; ++i) {
    if(mLamps[i] == 0) { mLamps[i] = l_rnr; return i; }
  }
  return -1;
}

void RnrDriver::ReturnLamp(int lamp)
{
  mLamps[lamp] = 0;
}

/**************************************************************************/
// RnrMods interface
/**************************************************************************/

void RnrDriver::SetDefRnrMod(FID_t fid, TObject* ud)
{
  if(find_rnrmod(fid)) {
    mRMI->second.def = ud;
  } else {
    mRnrMods[fid].def = ud;
  }
}

TObject* RnrDriver::GetDefRnrMod(FID_t fid)
{
  if(find_rnrmod(fid)) {
    return mRMI->second.def;
  } else {
    return 0;
  }
}

void RnrDriver::PushRnrMod(FID_t fid, TObject* ud)
{
  if(find_rnrmod(fid)) {
    mRMI->second.stack.push(ud);
  } else {
    mRnrMods[fid].stack.push(ud);
  }
}

TObject* RnrDriver::PopRnrMod(FID_t fid)
{
  if(find_rnrmod(fid) && ! mRMI->second.stack.empty()) {
    TObject* r = mRMI->second.stack.top();
    mRMI->second.stack.pop();
    return r;
  } else {
    return 0;
  }
}

TObject* RnrDriver::TopRnrMod(FID_t fid)
{
  if(find_rnrmod(fid) && ! mRMI->second.stack.empty()) {
    return mRMI->second.stack.top();
  } else {
    return 0;
  }
}

TObject* RnrDriver::GetRnrMod(FID_t fid)
{
  if(find_rnrmod(fid) == false) {
    mRMI = mRnrMods.insert(make_pair(fid, RnrMod())).first;
    mRMFid = fid;
  }

  if(mRMI->second.stack.empty()) {
    if(mRMI->second.def) return mRMI->second.def;
    if(mRMI->second.def_autogen == 0)
      mRMI->second.def_autogen = GledNS::ConstructLens(fid);
    return mRMI->second.def_autogen;
  } else {
    return mRMI->second.stack.top();
  }
}

/**************************************************************************/

void RnrDriver::RemoveRnrModEntry(FID_t fid)
{
  if(find_rnrmod(fid)) {
    delete mRMI->second.def_autogen;
    mRnrMods.erase(mRMI); mRMFid.clear();
  }
}

void RnrDriver::CleanUpRnrModDefaults()
{
  static const string _eh("RnrDriver::CleanUpRnrModDefaults ");

  for(hRnrMod_i i=mRnrMods.begin(); i!=mRnrMods.end(); ++i) {
    i->second.def = 0;
    int cnt = 0; 
    while( ! i->second.stack.empty()) {
      ++cnt;
      i->second.stack.pop();
    }
    if(cnt) printf("%sstack for '%s' not empty (%d entries remained).",
		   _eh.c_str(), GledNS::FindClassInfo(i->first)->fName.c_str());
  }
}

/**************************************************************************/

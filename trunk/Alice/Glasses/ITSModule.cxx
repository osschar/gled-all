// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ITSModule
//
//

#include "ITSModule.h"
#include "ITSModule.c7"

ClassImp(ITSModule)

/**************************************************************************/

void ITSModule::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  bSetTrans = true;
  mID   = -1;
  mInfo = 0;
}

/**************************************************************************/

ITSModule::ITSModule(Int_t id, ITSDigitsInfo* info) : ZNode() 
{
  _init();
  mInfo = info;
  if(mInfo) mInfo->IncRefCount();
  
  SetID(id);
}

ITSModule::~ITSModule()
{
  if(mInfo) mInfo->DecRefCount();
}

/**************************************************************************/

void ITSModule::SetID(Int_t id)
{
  static const string _eh("ITSModule::SetID ");

  if(mInfo == 0)
    throw(_eh + "ITSDigitsInfo not set.");

  if (id < mInfo->mGeom->GetStartSPD() || id > mInfo->mGeom->GetLastSSD())
    throw(_eh + GForm("%d is not valid. ID range from %d to %d", id,
		      mInfo->mGeom->GetStartSPD(), mInfo->mGeom->GetLastSSD()));

  mID = id;
  init_module();
  mStampReqTring = mStampReqTrans = Stamp(FID());
}

void ITSModule::init_module()
{
  mInfo->mGeom->GetModuleId(mID,mLayer,mLadder,mDet);

  if(mID < mInfo->mGeom->GetLastSPD()){
    mDetID = 0;
    mDx = mInfo->mSegSPD->Dx()*0.00005;
    mDz = 3.48; //  mInfo->mSegSPD->Dz()*0.00005;
    mDy = mInfo->mSegSPD->Dy()*0.00005;
  } else if (mID < mInfo->mGeom->GetLastSDD() ){
    mDetID = 1;
    mDx = mInfo->mSegSDD->Dx()*0.0001;
    mDz = mInfo->mSegSDD->Dz()*0.00005;
    mDy = mInfo->mSegSDD->Dy()*0.00005;
  }
  else {
    mDetID=2;
    mInfo->mSegSSD->SetLayer(mLayer);  
    mDx = mInfo->mSegSSD->Dx()*0.00005;
    mDz = mInfo->mSegSSD->Dz()*0.00005;
    mDy = mInfo->mSegSSD->Dy()*0.00005;
  }

  if(bSetTrans)
    set_trans();

  SetName(GForm("%d ITSModule",mID));
}

/**************************************************************************/

void ITSModule::set_trans()
{
  Double_t pos[3];
  Double_t rot[9];
  mInfo->mGeom->GetTrans(mID,pos);
  mInfo->mGeom->GetRotMatrix(mID,rot);
  mTrans.SetBaseV(1, rot[0], rot[1], rot[2]);
  mTrans.SetBaseV(2, rot[3], rot[4], rot[5]);
  mTrans.SetBaseV(3, rot[6], rot[7], rot[8]);
  mTrans.SetBaseV(4, pos[0], pos[1], pos[2]);
}

/**************************************************************************/

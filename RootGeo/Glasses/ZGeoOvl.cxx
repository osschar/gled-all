// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGeoOvl
//

#include "ZGeoOvl.h"
#include "ZGeoOvl.c7"

ClassImp(ZGeoOvl)

/**************************************************************************/

void ZGeoOvl::_init()
{
  mIsExtr  = false;
  mOverlap = 0;
  mPM_N = 0;
  mPM_p = 0;
  mRnrMark = true;
  mRnrNode = true;
}

/**************************************************************************/

void ZGeoOvl::Restore(TGeoVolume* vol)
{
  // Restore user data by reading mother node.

  TGeoNode* tn;
  ZGeoNode *zn;

  if(mIsExtr){
    tn = vol->FindNode(GetTitle());
    SetTNode(tn);
    AssertUserData();
  } else {
    zn = (ZGeoNode*)FrontElement();
    tn = vol->FindNode(zn->GetTitle());
    zn->SetTNode(tn);
    zn->AssertUserData();

    zn = (ZGeoNode*)BackElement();
    tn = vol->FindNode(zn->GetTitle());
    zn->SetTNode(tn);
    zn->AssertUserData();
  }
}

/**************************************************************************/

void ZGeoOvl::DumpOvl()
{
  const char *nn1, *nn2;
  if (mIsExtr) {
    nn1 = GetName();
    nn2 = 0;
  }
  else {
    ZGeoNode* n1 = (ZGeoNode*)FrontElement();
    ZGeoNode* n2 = (ZGeoNode*)BackElement();
    nn1 = n1->GetName();
    nn2 = n2->GetName();
  }
  printf("Extr:%d IsShown:%d Node1:%s Node2:%s Mother:%s %f\n",
	 mIsExtr ? 1:0, GetRnrSelf(), nn1, nn2, GetParent()->GetTitle(),
	 mOverlap);
}

// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGeoOvl
//
//

#include "ZGeoOvl.h"

#include "ZGeoOvl.c7"

ClassImp(ZGeoOvl)

/**************************************************************************/

void ZGeoOvl::_init()
{
  // !!!! Set all links to 0 !!!!
  mPM_p = 0;
  mIsExtr = false;
  mRnrMark = false;
}

/**************************************************************************/
void ZGeoOvl::SetRnrSelf(Bool_t rnrself)
{
  ZGeoNode::SetRnrSelf(rnrself);
  mRnrMark = rnrself;
  if (! mIsExtr) {
    ZGeoNode* n = (ZGeoNode*)First();
    n->SetRnrSelf(rnrself);
    n = (ZGeoNode*)Last();
    n->SetRnrSelf(rnrself);
  }
}

/**************************************************************************/

void ZGeoOvl::Dump()
{
  const char *nn1, *nn2;
  
  if (mIsExtr) {
    nn1 = GetName();
    nn2 = 0;
  }
  else {
    ZGeoNode* n1 = (ZGeoNode*)First();
    ZGeoNode* n2 = (ZGeoNode*)Last();
    nn1 = n1->GetName();
    nn2 = n2->GetName();
  }
  printf("Extr:%d IsShown:%d Node1:%s Node2:%s Mother:%s %f\n", mIsExtr?1:0, GetRnrSelf(), nn1, nn2, GetParent()->GetTitle(),  mOverlap);
}

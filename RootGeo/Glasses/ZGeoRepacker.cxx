// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGeoRepacker
//
//

#include "ZGeoRepacker.h"
#include "ZGeoRepacker.c7"

#include <TGeoManager.h>

ClassImp(ZGeoRepacker)

/**************************************************************************/

void ZGeoRepacker::_init()
{
  // *** Set all links to 0 ***
  mRoot = 0;
  mOutFile = "simple_geo.root";
}

/**************************************************************************/

void ZGeoRepacker::RepackGeometry()
{
  mSrcGeo = gGeoManager; // Perhaps should import it independently.

  mNeoGeo = new TGeoManager("Simplix", "Driplix");
  
  repack_geometry(mRoot, 0);

  mNeoGeo->CloseGeometry();
  mNeoGeo->Export(mOutFile.Data(), "Xilpmis");
  delete mNeoGeo;
  mNeoGeo = 0;
}

/**************************************************************************/

void ZGeoRepacker::repack_geometry(ZGeoNode* zgnode, TGeoVolume* parent_volume)
{
  bool recurse_p = false;
  TGeoNode*   tnode   = 0;
  TGeoVolume* tvolume = 0;
  TGeoShape*  tshape  = 0;

  tnode = zgnode->GetTNode();
  if(tnode == 0) {
    printf("Null node for %s; assuming it's a holder and descending.\n",
	   zgnode->Identify().c_str());

    recurse_p = true;
    tvolume = parent_volume;
    goto recurse_loop;
  }

  tvolume = tnode->GetVolume();
  if(tvolume == 0) {
    printf("Null volume for %s; skipping.\n", zgnode->Identify().c_str());
    return;
  }

  tshape  = tvolume->GetShape();
  if(tshape && HasShape(tshape) == false) {
    mNeoGeo->AddShape(tshape);
    PutShape(tshape);
  }
  // Materials, matrices?

  if(HasVolume(tvolume) == false) {
    tvolume->ClearNodes();
    tvolume->SetNtotal(0);
    mNeoGeo->AddVolume(tvolume);
    recurse_p = true;
  }
  PutVolume(tvolume);      
  if(parent_volume == 0) {
    mNeoGeo->SetTopVolume(tvolume);
  } else {
    parent_volume->AddNode(tvolume, GetVolume(tvolume), tnode->GetMatrix());
  }

 recurse_loop:

  if(recurse_p) {
    list<ZGeoNode*> l; zgnode->CopyByGlass<ZGeoNode*>(l);
    for(list<ZGeoNode*>::iterator i=l.begin(); i!=l.end(); ++i)
      repack_geometry(*i, tvolume);
  }
}


// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Menu
//
//

#include "Menu.h"
#include "Menu.c7"
#include <vector>

ClassImp(Menu)

/**************************************************************************/

void Menu::_init()
{
  // *** Set all links to 0 ***
  mParent = 0;
  mCanvas3D = 0;
  mCanvas2D = 0;
  mSettings = 0;

  mCreatorLens = 0;
  mOnEnterLens = 0;
  mOnExitLens = 0;

  mDigImport = false;
  mShowIn3D = true;
}

/**************************************************************************/

void Menu::OnEnter()
{
  printf("Menu::OnEnter %s canvas %p creator %p\n", 
  	 GetName(), *mCanvas3D, *mCreatorLens); 
  // Dump();
  if(mOnEnterLens != 0) {
    printf("Menu::OnEnterLens create mir \n");
    GledNS::MethodInfo* mi;
    mi = GledNS::DeduceMethodInfo(*mOnEnterLens, mOnEnterMethod.Data());
    if(mi == 0) return;
    ZMIR mir(*mOnEnterLens);
    mi->ImprintMir(mir);
    mSaturn->PostMIR(&mir);
  }


  if(mCanvas3D == 0) {
    mCanvas3D = new ZNode(GForm("%s 3Dcanvas"),GetName());
    mCanvas3D->SetRnrElements(false);
    mQueen->CheckIn(*mCanvas3D);
  }

  if(mCanvas2D == 0) {
    mCanvas2D = new ZNode(GForm("%s 2Dcanvas"),GetName());
    mCanvas2D->SetRnrElements(false);
    mQueen->CheckIn(*mCanvas2D);
  }

  if(mCreatorLens != 0 && (mDigImport==false)) {
    printf("%s Menu::Oncreate create mir %s \n",GetName(), mOnEnterMethod.Data());
    GledNS::MethodInfo* mi;
    mi = GledNS::DeduceMethodInfo(*mCreatorLens, mCreatorMethod.Data());
    if(mi == 0) return;
    ZMIR mir(*mCreatorLens, *mCanvas3D, *mCanvas2D);
    mi->ImprintMir(mir);
    mSaturn->PostMIR(&mir);
    mDigImport = true;
  }

 if(mOnExitLens != 0) {
    printf("Menu::OnExitLens create mir \n");
    GledNS::MethodInfo* mi;
    mi = GledNS::DeduceMethodInfo(*mOnExitLens, mOnExitMethod.Data());
    if(mi == 0) return;
    ZMIR mir(*mOnExitLens);
    mi->ImprintMir(mir);
    mSaturn->PostMIR(&mir);
  }
}

void Menu::Dump()
{
  Int_t count = 0;
  list<ZGlass*> dl3;
  if(mCanvas3D != 0) {
    mCanvas3D->CopyList(dl3);
    // printf("3D Cavas content: \n");
    for(list<ZGlass*>::iterator i=dl3.begin(); i!=dl3.end(); ++i) {
      printf("Menu 3D canvas %d %s \n", count, (*i)->GetName());
      ++count;
    }
  }
  if(mCanvas2D != 0) {
    count = 0;
    list<ZGlass*> dl2;
    mCanvas2D->CopyList(dl2);
    //   printf("2D Cavas content: \n");
    for(list<ZGlass*>::iterator i=dl2.begin(); i!=dl2.end(); ++i) {
      printf("Menu 2D canvas %d %s class %s \n", count, (*i)->GetName(),
	     (*i)->ClassName());
      ++count;
    }
  }
}

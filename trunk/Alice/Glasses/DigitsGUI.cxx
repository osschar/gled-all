// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// DigitsGUI
//
//

#include "DigitsGUI.h"
#include "DigitsGUI.c7"

#include <Glasses/WGlButton.h>

ClassImp(DigitsGUI)

/**************************************************************************/
  void DigitsGUI::_init()
{
  // *** Set all links to 0 ***
  mZAliLoad = 0;
  mDigits = 0;
  mTPCDigits = 0;
  mITSDigits = 0;

  // its
  mITSRM = 0;
  mITSSubDigits = 0;
  mITSMenu = 0;
  mITSSubMenu = 0;
  bSPDImport = false;
  bSDDImport = false;
  bSSDImport = false;

  //tpc
  mTPCMenu = 0;
  bTPCImport = false;

  // main
  mMainMenu = 0;
}

void  DigitsGUI::Init()
{
  // main menu button
  WGlButton* mbut = new WGlButton("MainMenu");
  mbut->Set3Pos(8.6, 5, 0);
  mbut->SetDx(1.2);
  mbut->SetCbackAlpha(this);
  mbut->SetCbackMethodName("MainMenu");
  mQueen->CheckIn(mbut);

  mDigits = new ZNode("Digits");
  mQueen->CheckIn(mDigits); mZAliLoad->Add(mDigits);
  /**************************************************************************/
  // START ITS MENU
  /**************************************************************************/
  // ITS Sub Menu
  // create holder for ITS digits and add it to ZAliload
  mITSDigits = new ZNode("ITS Digits holder");
  mQueen->CheckIn(mITSDigits); mDigits->Add(mITSDigits);

  mITSSubMenu = new ZNode("ITSSubMenu");
  mQueen->CheckIn(mITSSubMenu);

  WGlButton* but1 = new WGlButton("Scale -");
  but1->Set3Pos(8.6, 1, 0);
  but1->SetDx(1.2);
  but1->SetCbackAlpha(mITSRM);
  but1->SetCbackMethodName("ZoomIn");
  mQueen->CheckIn(but1);
  mITSSubMenu->Add(but1);

  WGlButton* but2 = new WGlButton("Scale +");
  but2->Set3Pos(8.6, 2, 0);
  but2->SetDx(1.2);
  but2->SetCbackAlpha(mITSRM);
  but2->SetCbackMethodName("ZoomOut");
  mQueen->CheckIn(but2);
  mITSSubMenu->Add(but2);

  WGlButton* but3 = new WGlButton("InnerLayer");
  but3->Set3Pos(8.6, 3, 0);
  but3->SetDx(1.2);
  but3->SetCbackAlpha(this);
  but3->SetCbackMethodName("InnerLayer");
  mQueen->CheckIn(but3);
  mITSSubMenu->Add(but3);

  WGlButton* but4 = new WGlButton("OuterLayer");
  but4->Set3Pos(8.6, 4, 0);
  but4->SetDx(1.2);
  but4->SetCbackAlpha(this);
  but4->SetCbackMethodName("OuterLayer");
  mQueen->CheckIn(but4);
  mITSSubMenu->Add(but4);
  mITSSubMenu->Add(mbut);

  mITSSubMenu->RnrOffForDaughters();

  //ITS Main Menu
  mITSMenu = new ZNode("ITSMenu");
  mQueen->CheckIn(mITSMenu);

  WGlButton* spdbut = new WGlButton("SPD");
  spdbut->Set3Pos(8.6, 1, 0);
  spdbut->SetDx(1.2);
  spdbut->SetCbackAlpha(this);
  spdbut->SetCbackMethodName("SPDMenu");
  mQueen->CheckIn(spdbut);
  mITSMenu->Add(spdbut);

  WGlButton* sddbut = new WGlButton("SDD");
  sddbut->Set3Pos(8.6, 2, 0);
  sddbut->SetDx(1.2);
  sddbut->SetCbackAlpha(this);
  sddbut->SetCbackMethodName("SDDMenu");
  mQueen->CheckIn(sddbut);
  mITSMenu->Add(sddbut);

  WGlButton* ssdbut = new WGlButton("SSD");
  ssdbut->Set3Pos(8.6, 3, 0);
  ssdbut->SetDx(1.2);
  ssdbut->SetCbackAlpha(this);
  ssdbut->SetCbackMethodName("SSDMenu");
  mQueen->CheckIn(ssdbut);
  mITSMenu->Add(ssdbut);
  mITSMenu->Add(mbut);
  mITSMenu->RnrOffForDaughters();

  mITSMenu->Add(mITSSubMenu);
  /**************************************************************************/
  // START TPC MENU
  /**************************************************************************/
  mTPCMenu = new ZNode("TPCMenu");
  mQueen->CheckIn(mTPCMenu);

  mTPCMenu->Add(mbut);
  mTPCDigits = new ZNode("TPC Digits");
  mQueen->CheckIn(mTPCDigits); mDigits->Add(mTPCDigits);
  mTPCMenu->RnrOffForDaughters();

  /**************************************************************************/
  // START MAIN MENU 
  /**************************************************************************/
  mMainMenu = new ZNode("MainMenu");
  mQueen->CheckIn(mMainMenu);
  Add(mMainMenu);

  WGlButton* itsbut = new WGlButton("ITS");
  itsbut->Set3Pos(2.6, 1, 0);
  itsbut->SetDx(1.2);
  itsbut->SetCbackAlpha(this);
  itsbut->SetCbackMethodName("ITSMenu");
  mQueen->CheckIn(itsbut);
  mMainMenu->Add(itsbut);

  mITSSubMenu->Add(itsbut);

  WGlButton* tpcbut = new WGlButton("TPC");
  tpcbut->Set3Pos(8.6, 2, 0);
  tpcbut->SetDx(1.2);
  tpcbut->SetCbackAlpha(this);
  tpcbut->SetCbackMethodName("TPCMenu");
  mQueen->CheckIn(tpcbut);

  mMainMenu->Add(tpcbut);
  mMainMenu->Add(mTPCMenu);
  mMainMenu->Add(mITSMenu);

  MainMenu();
}

/**************************************************************************/
void  DigitsGUI::SPDMenu() {
  mMainMenu->RnrOffForDaughters();
  mITSMenu->RnrOffForDaughters();
  mITSSubMenu->SetRnrSelf(true);
  mITSSubMenu->RnrOnForDaughters();

mITSDigits->RnrElmsOffForDaughters();
  if( !bSPDImport) {
    mZAliLoad->ShowITSDet(0,1, mITSDigits);
    bSPDImport = true;
  }
  mITSSubDigits = show_this_lens("SPD",mITSDigits );
}

void  DigitsGUI::SDDMenu() {
  mMainMenu->RnrOffForDaughters();
  mITSMenu->RnrOffForDaughters();
  mITSSubMenu->SetRnrSelf(true);
  mITSSubMenu->RnrOnForDaughters();

  mITSDigits->RnrElmsOffForDaughters();
  if( !bSDDImport) {
    mZAliLoad->ShowITSDet(1,1, mITSDigits);
    bSDDImport = true;
  }
  mITSSubDigits = show_this_lens("SDD",mITSDigits );
}

void  DigitsGUI::SSDMenu() {
  mMainMenu->RnrOffForDaughters();
  mITSMenu->RnrOffForDaughters();
  mITSSubMenu->SetRnrSelf(true);
  mITSSubMenu->RnrOnForDaughters();

  mITSDigits->RnrElmsOffForDaughters();
  if( !bSSDImport) {
    mZAliLoad->ShowITSDet(2,1, mITSDigits);
    bSSDImport = true;
  }
  mITSSubDigits = show_this_lens("SSD",mITSDigits );
}

void DigitsGUI::ITSMenu(){
  mMainMenu->RnrOffForDaughters();
  mITSSubMenu->RnrOffForDaughters();
  mITSMenu->SetRnrSelf(true);
  mITSMenu->RnrOnForDaughters();

  mITSDigits->SetRnrElements(true);
}

void  DigitsGUI::InnerLayer(){
  if(mITSSubDigits == 0) return;
  ZNode* lens = dynamic_cast<ZNode*>(mITSSubDigits->FindLensByPath("InnerLayer"));
  Bool_t rnr = lens->GetRnrSelf();
  lens->SetRnrSelf(!rnr);
  lens->SetRnrElements(!rnr);
}

void  DigitsGUI::OuterLayer(){
  if(mITSSubDigits == 0) return;
  ZNode* lens = dynamic_cast<ZNode*>(mITSSubDigits->FindLensByPath("OuterLayer"));
  Bool_t rnr = lens->GetRnrSelf();
  lens->SetRnrSelf(!rnr);
  lens->SetRnrElements(!rnr);
}

/**************************************************************************/
void DigitsGUI::TPCMenu(){
  mMainMenu->RnrOffForDaughters();
  mTPCMenu->RnrOnForDaughters();

  if( !bTPCImport) {
    mZAliLoad->ShowTPCPlate(-1, mTPCDigits);
    bTPCImport = true;
  }
  mTPCDigits->SetRnrElements(true);
} 

/**************************************************************************/
void DigitsGUI::MainMenu(){
  mITSSubMenu->RnrOffForDaughters();
  mITSMenu->RnrOffForDaughters();
  mTPCMenu->RnrOffForDaughters();
  mMainMenu->RnrOnForDaughters();

  mITSDigits->SetRnrElements(false);
  mTPCDigits->SetRnrElements(false);
}

/**************************************************************************/
ZNode* DigitsGUI::show_this_lens(const string& url, ZNode* holder)
{
  holder->RnrOffForDaughters();
  holder->FindLensByPath(url);

  ZNode* lens = dynamic_cast<ZNode*>(holder->FindLensByPath(url));
  if(lens) {
    lens->SetRnrElements(true);
  }
  return lens;
}

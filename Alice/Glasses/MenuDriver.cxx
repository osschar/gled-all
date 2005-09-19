// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MenuDriver
//
//

#include "MenuDriver.h"
#include "MenuDriver.c7"
#include <Glasses/ZQueen.h> 
#include <Glasses/UINodeLink.h>

#include <Glasses/ITSScaledModule.h>
#include <Glasses/TPCSegFrame.h>
#include <Glasses/Rect.h>
#include <Glasses/WGlButton.h>
#include <Stones/SGridStepper.h>

ClassImp(MenuDriver)

/**************************************************************************/

  void MenuDriver::_init()
{
  // *** Set all links to 0 **
  mZAliLoad = 0;

  mOverlay = 0;
  mCurrent = 0;
  mPrevious = 0;
  mCanvas3D = 0;
  mCanvas2D = 0;
  mMDir = 0;
  
  mTPCPlateRM = 0;
  mTPCSegRM = 0;  

  mITSRM = 0;
}

/**************************************************************************/
void MenuDriver::Init()
{
  mScreenText = new ScreenText("title of current menu");
  mScreenText->SetX(-4);
  mScreenText->SetFgCol(.7,.7,.7,1.);
  mQueen->CheckIn(mScreenText);
  mZAliLoad->Add(mScreenText);

  mContents = new ZList("WGl Contents");
  mQueen->CheckIn(mContents);
  Add(mContents);

  mMDir = new ZNode("Settings");
  mQueen->CheckIn(mMDir);
  mOverlay->Add(mMDir);
  
  mCanvas3D = new ZNode("MDriver 3DCanvas");
  mQueen->CheckIn(mCanvas3D);
  mZAliLoad->Add(mCanvas3D);

  mCanvas2D = new ZNode("MDriver 2DCanvas");
  mQueen->CheckIn(mCanvas2D);
  mOverlay->Add(mCanvas2D);


  // RNR STYLES 
  mTPCPlateRM = new TPCSegRnrMod("TPC Plate RM"); mQueen->CheckIn(mTPCPlateRM); 
  mTPCSegRM = new TPCSegRnrMod("TPC Seg RM");     mQueen->CheckIn(mTPCSegRM);
  mTPCSegRM->SetFrameCol(1.,0.,0.,1.);
  mZAliLoad->Add(mTPCPlateRM);
  mZAliLoad->Add(mTPCSegRM);

  mITSRM = new ITSDigRnrMod(); mQueen->CheckIn(mITSRM);
  mITSRM->SetFrameCol(1.,1.,1.,0.6);
  mZAliLoad->Add(mITSRM);

 

  // TPC
  Menu* tpcseg = new Menu("TPCSeg"); mQueen->CheckIn(tpcseg);
  tpcseg->SetShowIn3D(false); 
  Menu* tpc = new Menu("TPC"); mQueen->CheckIn(tpc);
  tpc->SetShowIn3D(false);
  tpc->Add(tpcseg);
  tpcseg->SetParent(tpc);

  // ITS
  Menu* itsmod = new Menu("ITSMod"); mQueen->CheckIn(itsmod);
  // TEST
  ZList* settings = new ZList("settings");
  mQueen->CheckIn(settings);
  WGlButton* zoomin = new WGlButton("ZoomIn");
  mQueen->CheckIn(zoomin);
  zoomin->SetCbackAlpha(mITSRM);
  zoomin->SetCbackMethodName("ZoomIn");
  settings->Add(zoomin);
  WGlButton* zoomout = new WGlButton("ZoomOut");
  mQueen->CheckIn(zoomout);
  zoomout->SetCbackAlpha(mITSRM);
  zoomout->SetCbackMethodName("ZoomOut");
  settings->Add(zoomout);
  itsmod->SetSettings(settings);

  Menu* spdl1 =  new Menu("InnerLayer"); mQueen->CheckIn(spdl1);
  spdl1->SetShowIn3D(false);
  spdl1->Add(itsmod); 
  Menu* spdl2 =  new Menu("OuterLayer"); mQueen->CheckIn(spdl2);
  spdl2->Add(itsmod);
  spdl2->SetShowIn3D(false);

  Menu* sddl1 =  new Menu("InnerLayer"); mQueen->CheckIn(sddl1);
  sddl1->SetShowIn3D(false);
  sddl1->Add(itsmod); 
  Menu* sddl2 =  new Menu("OuterLayer"); mQueen->CheckIn(sddl2);
  sddl2->Add(itsmod);
  sddl2->SetShowIn3D(false);

  Menu* ssdl1 =  new Menu("InnerLayer"); mQueen->CheckIn(ssdl1);
  ssdl1->SetShowIn3D(false);
  ssdl1->Add(itsmod); 
  Menu* ssdl2 =  new Menu("OuterLayer"); mQueen->CheckIn(ssdl2);
  ssdl2->Add(itsmod);
  ssdl2->SetShowIn3D(false);


  Menu* spd = new Menu("ITSSPD"); mQueen->CheckIn(spd);
  spd->Add(spdl1); spd->Add(spdl2);
  Menu* sdd = new Menu("ITSSDD"); mQueen->CheckIn(sdd);
  sdd->Add(sddl1); sdd->Add(sddl2);
  Menu* ssd = new Menu("ITSSSD"); mQueen->CheckIn(ssd);
  ssd->Add(ssdl1); ssd->Add(ssdl2);
  Menu* its = new Menu("ITS"); mQueen->CheckIn(its);
  its->Add(spd);its->Add(sdd);its->Add(ssd);  
  spd->SetParent(its);
  sdd->SetParent(its);
  ssd->SetParent(its);

  Menu* main = new Menu("Main"); mQueen->CheckIn(main);
  Add(main);
  Menu* exit = new Menu("Exit"); mQueen->CheckIn(exit);
  main->Add(its); main->Add(tpc); main->Add(exit);
  its->SetParent(main);
  tpc->SetParent(main);
  exit->SetParent(main);

  // set callbacks
  itsmod->SetCreatorLens(this);
  itsmod->SetCreatorMethod("ITSModuleMenu");
  itsmod->SetShowIn3D(false);


  spdl1->SetCreatorLens(this);
  spdl1->SetCreatorMethod("SelectITSLayer");
  spdl2->SetCreatorLens(this);
  spdl2->SetCreatorMethod("SelectITSLayer");
  spdl1->SetOnEnterLens(this);
  spdl1->SetOnEnterMethod("SetParentToSubMenues");
  spdl2->SetOnEnterLens(this);
  spdl2->SetOnEnterMethod("SetParentToSubMenues");

  sddl1->SetCreatorLens(this);
  sddl1->SetCreatorMethod("SelectITSLayer");
  sddl2->SetCreatorLens(this);
  sddl2->SetCreatorMethod("SelectITSLayer");
  sddl1->SetOnEnterLens(this);
  sddl1->SetOnEnterMethod("SetParentToSubMenues");
  sddl2->SetOnEnterLens(this);
  sddl2->SetOnEnterMethod("SetParentToSubMenues");

  ssdl1->SetCreatorLens(this);
  ssdl1->SetCreatorMethod("SelectITSLayer");
  ssdl2->SetCreatorLens(this);
  ssdl2->SetCreatorMethod("SelectITSLayer");
  ssdl1->SetOnEnterLens(this);
  ssdl1->SetOnEnterMethod("SetParentToSubMenues");
  ssdl1->SetOnEnterLens(this);
  ssdl2->SetOnEnterLens(this);
  ssdl2->SetOnEnterMethod("SetParentToSubMenues");

  spd->SetCreatorLens(this);
  spd->SetCreatorMethod("SPDMenu");
  spd->SetOnEnterLens(this);
  spd->SetOnEnterMethod("SetParentToSubMenues");
  sdd->SetCreatorLens(this);
  sdd->SetCreatorMethod("SDDMenu");
  sdd->SetOnEnterLens(this);
  sdd->SetOnEnterMethod("SetParentToSubMenues");
  ssd->SetCreatorLens(this);
  ssd->SetCreatorMethod("SSDMenu");
  ssd->SetOnEnterLens(this);
  ssd->SetOnEnterMethod("SetParentToSubMenues");

  its->SetCreatorLens(this);
  its->SetCreatorMethod("ITSMenu");

  tpc->SetCreatorLens(this);
  tpc->SetCreatorMethod("TPCMenu");
  tpcseg->SetCreatorLens(this);
  tpcseg->SetCreatorMethod("TPCSegMenu");


  exit->SetOnEnterLens(this);
  exit->SetOnEnterMethod("Exit");

  mCurrent = main; mPrevious=main;
  SwitchSelected(main);
}

																									  /**************************************************************************/
/**************************************************************************/
void MenuDriver::SwitchSelected(Menu* menu)
{
  //  printf("SwitchSelected menu %s current %s prev %s\n", 
  //	 menu->GetName(), mCurrent->GetName(), mPrevious->GetName());

  mPrevious = mCurrent;
  mCurrent = menu;

  mScreenText->SetText(GForm("%s Menu",menu->GetName()));
  mContents->ClearList();
  mMDir->ClearList(); // erase menu settings
  mCanvas3D->ClearList();
  mCanvas2D->ClearList();

  // GL menu GUI
  list<Menu*> ml;  
  menu->CopyByGlass<Menu*>(ml);
  for(list<Menu*>::iterator i=ml.begin(); i!=ml.end(); ++i) {
    mContents->Add(*i);
  }
  if(menu->GetParent()) {
    mContents->Add(menu->GetParent());
  }

  if(menu->GetShowIn3D()){
    mCanvas3D->SetRnrElements(true);
    mCanvas2D->SetRnrElements(false);
  }else {
    mCanvas3D->SetRnrElements(false);
    mCanvas2D->SetRnrElements(true);
  }

  // canvas
  // 
  menu->OnEnter();
  //copy content to the menu settigs wgl directory
  if(menu->GetSettings() && (!menu->GetSettings()->IsEmpty())){
    list<ZGlass*> cl;  
    menu->GetSettings()->Copy(cl);
    SGridStepper stepper(SGridStepper::SM_YXZ);
    stepper.SetNs(6, 2);
    stepper.SetDs(-2, 1);
    for(list<ZGlass*>::iterator i=cl.begin(); i!=cl.end(); ++i) {
      ZNode* n = dynamic_cast<ZNode*>(*i);
      if(n != 0) {
	Float_t pos[3];
	stepper.GetPosition(pos);
	pos[0] += 7.8; pos[1] += 6;
	n->Set3Pos(pos[0], pos[1], 0.5);
	stepper.Step();
      }
      mMDir->Add(*i);
    }
  }
  if(menu->GetCanvas3D()){
    //printf("SwitchSelected read menu canvas %s \n",menu->GetCanvas3D()->GetName() );
    list<ZGlass*> cl;  
    menu->GetCanvas3D()->Copy(cl);
    for(list<ZGlass*>::iterator i=cl.begin(); i!=cl.end(); ++i) {
      //printf("Fill canvas %s from %s \n", (*i)->GetName(), menu->GetName());
      mCanvas3D->Add(*i);
    }
  }

  if(menu->GetCanvas2D() && (!menu->GetCanvas2D()->IsEmpty())){
    //printf("SwitchSelected read menu canvas %s \n",menu->GetCanvas2D()->GetName() );
    list<ZGlass*> cl;  
    menu->GetCanvas2D()->Copy(cl);
    for(list<ZGlass*>::iterator i=cl.begin(); i!=cl.end(); ++i) {
      //printf("Fill LINKS  %s from %s \n", (*i)->GetName(), menu->GetName());
      mCanvas2D->Add(*i);
    }
  }
}

/**************************************************************************/
void MenuDriver::SetParentToSubMenues(Menu* menu)
{
  // set parent to layer1 and layer2
  //  printf("SetParentToSubMenues \n");
  if(menu == 0) menu = mCurrent;
  list<Menu*> sml;  
  mCurrent->CopyByGlass<Menu*>(sml);
  for(list<Menu*>::iterator i=sml.begin(); i!=sml.end(); ++i) {
    (*i)->SetParent(mCurrent);
  } 
}

/**************************************************************************/
void MenuDriver::Exit() 
{
  system("killall -9 gled");
}
/**************************************************************************/

void MenuDriver::ITSMenu(ZNode* dh, ZNode* lh) 
{
  mZAliLoad->ShowITSDet(0,1, dh);
  mZAliLoad->ShowITSDet(1,1, dh);
  mZAliLoad->ShowITSDet(2,1, dh);
}

void MenuDriver::SPDMenu(ZNode* dh, ZNode* lh) 
{
  // printf("MenuDriver::SPD  previous %s\n", mPrevious->GetName());
  ZNode* lens = dynamic_cast<ZNode*>(mPrevious->GetCanvas3D()->FindLensByPath("SPD"));
  if(lens) {
    list<ZNode*> cl;  
    lens->CopyByGlass<ZNode*>(cl);
    for(list<ZNode*>::iterator i=cl.begin(); i!=cl.end(); ++i) {
      dh->Add(*i);
    } 
  }
}

void MenuDriver::SDDMenu(ZNode* dh, ZNode* lh) 
{
  ZNode* lens = dynamic_cast<ZNode*>(mPrevious->GetCanvas3D()->FindLensByPath("SDD"));
  if(lens) {
    list<ZNode*> cl;  
    lens->CopyByGlass<ZNode*>(cl);
    for(list<ZNode*>::iterator i=cl.begin(); i!=cl.end(); ++i) {
      dh->Add(*i);
    } 
  }
}

void MenuDriver::SSDMenu(ZNode* dh, ZNode* lh) 
{
  ZNode* lens = dynamic_cast<ZNode*>(mPrevious->GetCanvas3D()->FindLensByPath("SSD"));
  if(lens) {
    list<ZNode*> cl;  
    lens->CopyByGlass<ZNode*>(cl);
    for(list<ZNode*>::iterator i=cl.begin(); i!=cl.end(); ++i) {
      dh->Add(*i);
    } 
  }
}

void MenuDriver:: SelectITSLayer(ZNode* dh, ZNode* lh) 
{
  printf("MenuDriver:: SelectLayer current %s previous %s\n", mCurrent->GetName(),mPrevious->GetName());
  AliITSgeom* geo = mZAliLoad->GetITSDigInfo()->mGeom;
  ITSScaledModule* mod;
  UINodeLink* link;
  Float_t scale, x, y, x0, y0;
  x0 = 1.; 
  y0 = 1.;
  Int_t lad;
  Float_t gridH = 7.;

  Rect* plane = new Rect("BasePlane");
  plane->SetColor(1.,1.,1.,0.23);
  mQueen->CheckIn(plane);
  lh->Add(plane);
 
  Bool_t inner = false;

  Int_t lay;
  Int_t nLad, nCol; 
  if(strcmp(mCurrent->GetName(),"InnerLayer") == 0) inner = true;


  if(strcmp(mPrevious->GetName(), "ITSSPD") == 0) {
    if (inner) lay = 1;
    else lay = 2;
  }
  else if(strcmp(mPrevious->GetName(), "ITSSDD") == 0) {
    if (inner) lay = 3;
    else lay = 4;
  }
  else {
    if (inner) lay = 5;
    else lay = 6;
  }
  nLad = geo->GetNladders(lay);
  nCol = geo->GetNdetectors(lay);

  ZNode* lens = dynamic_cast<ZNode*>(mPrevious->GetCanvas3D()->FindLensByPath(mCurrent->GetName()));
  list<ITSScaledModule*> cl;  
  lens->CopyByGlass<ITSScaledModule*>(cl);
  for(list<ITSScaledModule*>::iterator i=cl.begin(); i!=cl.end(); ++i) {
    mod = *i;
    link = new UINodeLink(GForm("ITS mod link  %d",mod->GetID()));
    link->SetLens(mod);
    link->SetUseScale(true);
    scale = gridH/(nLad*2*mod->GetDx());
    link->SetScales(scale, scale, scale);
    link->RotateLF(1,2, TMath::Pi()/2);
    link->RotateLF(2,3, TMath::Pi()/2);
    if (lad != mod->GetLadder()) {
      lad = mod->GetLadder();
      y = y0 + (lad-1)*2*mod->GetDx()*scale + mod->GetDx()*scale;
    }
    x = x0 + (mod->GetDet()-1)*2*mod->GetDz()*scale + mod->GetDz()*scale;
    link->Set3Pos(x,y, 0.);
    link->SetCbackAlpha(this);link->SetCbackBeta(mod);
    link->SetCbackMethodName("ITSModPickMenu");
    mQueen->CheckIn(link);
    lh->Add(link);
    dh->Add(*i);
  }
  plane->SetVLen(gridH); 
  plane->SetULen(gridH*nCol*mod->GetDz()/(mod->GetDx()*nLad)); 
  plane->SetVStrips(nLad);
  plane->SetUStrips(nCol);
  plane->Set3Pos(plane->GetULen()/2.+x0, gridH/2.+y0, 0.);
}

void MenuDriver::ITSModuleMenu(ZNode* dh, ZNode* lh) 
{
  UINodeLink* ml = new UINodeLink("ITSModuleLink");
  ml->SetRnrMod(mITSRM);
  ml->SetUseScale(true);
  ml->SetScales(.5,.5,.5);
  ml->RotateLF(1,2, TMath::Pi()/2);
  ml->RotateLF(2,3, TMath::Pi()/2); 
  ml->Move3(3.5,4.,0.);
  mQueen->CheckIn(ml);
  // if no segment selected, then show first one == Seg0
  ZGlass* mod = mPrevious->GetCanvas3D()->First();
  ml->SetLens(mod);
  lh->Add(ml);
}


// calback from segm pick
void MenuDriver::ITSModPickMenu(ZGlass* lens){
  // printf("ITSModPickMenu \n");
  Menu* modm = dynamic_cast<Menu*>(mCurrent->FindLensByPath("ITSMod"));

  if(modm->GetDigImport() == false){
    modm->OnEnter();
  }
  UINodeLink*  ml = dynamic_cast<UINodeLink*>(modm->GetCanvas2D()->FindLensByPath("ITSModuleLink"));
  ml->SetLens(lens);
  SwitchSelected(modm);
}

/**************************************************************************/
void MenuDriver::SwitchViewMenu(Menu* menu){
  if(menu->GetShowIn3D()){
    menu->SetShowIn3D(false);
    menu->SetName("3DView");
  }else {
    menu->SetShowIn3D(true);
    menu->SetName("2DView");
  }
}
/**************************************************************************/

void MenuDriver::TPCMenu(ZNode* dh, ZNode* lh) 
{
  TPCSegment* seg;
  UINodeLink* link;
  for (int i=0; i < 36; i++) {
    seg = mZAliLoad->ShowTPCSegment(i, dh);
    link = new UINodeLink(GForm("Segment link  %d",i));
    link->SetRnrMod(mTPCPlateRM);
    link->SetLens(seg);
    link->RotateLF(1,2, - TMath::Pi()/2 + (i + 0.5)*20*TMath::Pi()/180);
    link->RotateLF(1,3, TMath::Pi());
    link->SetUseScale(true);
    link->SetScales(0.008, 0.008, 0.008);
    if(i < 18)
      link->Move3(2.4,6.2,0.); 
    else
      link->Move3(7.3, 6.2,0.);
    link->SetCbackAlpha(this);link->SetCbackBeta(seg);
    link->SetCbackMethodName("TPCSegPickMenu");
    link->SetFocusAlpha(this);link->SetFocusBeta(link);
    link->SetFocusMethodName("TPCSegFocus");

    mQueen->CheckIn(link);
    lh->Add(link);
  }
}

// callback from press button
void MenuDriver::TPCSegMenu(ZNode* dh, ZNode* lh) 
{
  printf("MenuDriver::TPCSegMenu \n");
  Float_t scale = 0.05;
  Float_t x0 = 2.3;
  Float_t y0 = -3.3;
  
  UINodeLink* sl = new UINodeLink("SegmentLink");
  sl->SetRnrMod(mTPCSegRM);
  sl->SetUseScale(true);
  sl->SetScales(scale, scale, 1.);
  sl->Move3(x0,y0,0.);
  sl->SetActive(false);
  mQueen->CheckIn(sl);

  ZGlass* lens = mPrevious->GetCanvas3D()->First();
  TPCSegment* seg = dynamic_cast<TPCSegment*>(lens);
  sl->SetLens(seg);
  lh->Add(sl);

  TPCPadRow* pr = new TPCPadRow("TPCPadRow"); 
  pr->SetSegment(seg);
  pr->SetUseScale(true);
  pr->SetScales(0.06, 0.1, 1.);
  pr->Move3(7.5, 7.3, 0.2);
  mQueen->CheckIn(pr);
  lh->Add(pr);

  // render frame to get padrow selection
  TPCSegFrame* frame = new TPCSegFrame("TPCSegFrame");
  frame->SetTPCPar(mZAliLoad->GetTPCDigInfo()->mParameter);
  frame->SetUseScale(true);
  frame->SetScales(scale, scale, 1.);
  frame->Move3(x0,y0,0.2);
  frame->SetPadRow(pr);
  frame->SetLens(pr);
  mQueen->CheckIn(frame);
  lh->Add(frame);
}


void MenuDriver::TPCSegPickMenu(ZGlass* lens){
  printf("TPCSegPickMenu current %s lens %s \n", mCurrent->GetName(), lens->GetName());
 
  Menu* segm = dynamic_cast<Menu*>(mCurrent->FindLensByPath("TPCSeg"));
  UINodeLink* sl;

  if(segm->GetDigImport() == false){
    segm->OnEnter();
  }
 
  TPCSegment* seg =  dynamic_cast<TPCSegment*>(lens);
  printf("PCSegPickMenu seg %p class %s\n",seg, lens->ClassName());   
  sl = dynamic_cast<UINodeLink*>(segm->GetCanvas2D()->FindLensByPath("SegmentLink"));
  sl->SetLens(seg);
  sl->RotateLF(1,3, TMath::Pi());

  TPCPadRow* pr  = dynamic_cast<TPCPadRow*>(segm->GetCanvas2D()->FindLensByPath("TPCPadRow"));
  pr->SetSegment(seg);
  pr->RotateLF(1,3, TMath::Pi());

  SwitchSelected(segm);
}

void MenuDriver::TPCSegFocus(ZGlass* zlink)
{
  UINodeLink* link = dynamic_cast<UINodeLink*>(zlink);
  //printf("TPCSegFocus %s focus %d \n",lens->GetName(),link->GetHasFocus() ? 1:0 );
  if(link->GetHasFocus()) {
    //    lens->SetRnrMod(mTPCSegRM);
    link->SetRnrMod(mTPCSegRM);
  }
  else {
    // lens->SetRnrMod(mTPCPlateRM);
    link->SetRnrMod(mTPCPlateRM);
  }
}

/**************************************************************************/
void MenuDriver::Dump()
{
  Int_t count = 0;
  list<ZGlass*> dl3;
  mCanvas3D->Copy(dl3);
  //  printf("MenuDrive 3D Cavas content: \n");
  for(list<ZGlass*>::iterator i=dl3.begin(); i!=dl3.end(); ++i) {
    printf("MenuDrive 3D canvas %d %s \n", count,(*i)->GetName());
    count ++;
  }
  
  count = 0;
  list<ZGlass*> dl2;
  mCanvas2D->CopyByGlass<ZGlass*>(dl2);
  // printf("2D Cavas content: \n");
  for(list<ZGlass*>::iterator i=dl2.begin(); i!=dl2.end(); ++i) {
    printf("MenuDriver  2D canvas %d %s \n", count,(*i)->GetName());
    count ++;
  }
}

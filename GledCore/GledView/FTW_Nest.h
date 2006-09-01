// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_Nest_H
#define GledCore_FTW_Nest_H

#include <Gled/GledTypes.h>
class NestInfo;

#include <Eye/OptoStructs.h>
#include "FTW_SubShell.h"
#include "FTW.h"
#include "MTW_Layout.h"
class FTW_Leaf;
class FTW_Nest;

#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_ScrollPack.H>
#include <FL/Fl_SWM.H>
#include <FL/Fl_Group.H>
class Fl_Input; class Fl_Output; class Fl_Menu_Item;
class Fl_Box;

class FooPack;

/**************************************************************************/
// FTW_Nest
/**************************************************************************/

class FTW_Nest : public FTW_SubShell,
                 public OptoStructs::A_View,
                 public Fl_Window, public Fl_SWM_Client
{
  friend class FTW_Shell;

public:
  enum TargetType_e { TT_Point = 2, TT_Mark, TT_BelowMouse };

private:
  typedef map<FTW::Locator*, list<FTW::LocatorConsumer*> > mpLoc2pLCon_t;
  typedef map<FTW::Locator*, list<FTW::LocatorConsumer*> >::iterator	mpLoc2pLCon_i;

  void _build(int w, int h);
  void _finalize_build();

protected:
  NestInfo*	 mNestInfo;	// X{g}

  int		 mW;

  FTW::Locator	 mPoint;	// X{r}
  FTW::Locator	 mMark;		// X{r}
  FTW::Locator	 mBelowMouse;	// X{r}
  FTW::Locator*	 mTargetLoc;	// X{r}
  TargetType_e	 mTargetType;
  mpLoc2pLCon_t	 mConsumerMap;
  

  Fl_Pack*	 wMainPack;
  Fl_Pack*	 wMenuPack;
  Fl_Pack*	 wTargetPack;
  Fl_ScrollPack* mPack;         // X{g}

  bool		bLinksShown;	// X{g}
  bool		bCustomWeedsCreated;
  bool		bAntsReversed;	// X{g}

  MTW_Layout*	pLayout;	// X{g}
  Fl_Pack*	wMidPack;
  Fl_Box*	wSepBox;
  Fl_Box*	wCustomTitle;
  Fl_Group*	wCustomLabels;

  Fl_Output*	wInfoBar;	// X{g}

  void create_custom_weeds();
  void show_custom_weeds();
  void hide_custom_weeds();
  void wipe_custom_weeds();

public:

  static FTW_Nest* Create_FTW_Nest(FTW_Shell* sh, OptoStructs::ZGlassImg* img);

  FTW_Nest(FTW_Shell* sh, OptoStructs::ZGlassImg* img, int w=64, int h=30);
  FTW_Nest(FTW_Shell* sh, OptoStructs::ZGlassImg* img, int x, int y, int w, int h);
  ~FTW_Nest();

  virtual void AbsorbRay(Ray& ray);

  virtual void Rewidth();
  virtual void AdInsertio(FTW_Leaf* newleaf);
  virtual void SetSWM(Fl_SWM_Manager* swm, bool self_p=true);

  void InsertLeaf(FTW_Leaf* newleaf, int at);
  void InsertLeaf(FTW_Leaf* newleaf, FTW_Leaf* before=0);
  void RemoveLeaf(FTW_Leaf* exleaf);

  int  PackPos(FTW_Leaf* leaf);
  int  PackPosAfter(FTW_Leaf* leaf);
  int  PackPosBefore(FTW_Leaf* leaf);
  FTW_Leaf* LeafAt(int pos);

  /**************************************************************************/

  FTW_Leaf* VisibleLeafBefore(FTW_Leaf* leaf);
  FTW_Leaf* VisibleLeafAfter(FTW_Leaf* leaf);
  FTW_Leaf* FirstVisibleLeaf();
  FTW_Leaf* LastVisibleLeaf();

  /**************************************************************************/

  void LinksView();
  void CustomView();
  void EnactLayout(const char* layout=0);

  /**************************************************************************/

  void MigrateLocator(FTW::Locator& loc);
  void LocatorChange(FTW::Locator& loc);
  void ExchangePointAndMark();
  void TargetTypeChange(TargetType_e tt);
  void RegisterLocatorConsumer  (FTW::LocatorConsumer* lcon);
  void UnregisterLocatorConsumer(FTW::LocatorConsumer* lcon);
  bool IsPoint(FTW::Locator& loc) { return &loc == &mPoint; }
  bool IsMark (FTW::Locator& loc) { return &loc == &mMark;  }

  /**************************************************************************/

  void SetInfoBar(const char* info);
  void SetInfoBar(const TString& info) { SetInfoBar(info.Data()); }

  void ReverseAnts();

  /**************************************************************************/

  Fl_Color LeafName_Color(FTW_Leaf* leaf);
  Fl_Color AntName_Color(FTW_Ant* ant);

  virtual void label_window(const char* l=0);
  virtual int  handle(int ev);
  virtual void resize(int x, int y, int w, int h);

  /**************************************************************************/

  static const int def_W;
  static const int def_H;

  static const int min_W;
  static const int max_W;
  static const int min_H;
  static const int max_H;

#include "FTW_Nest.h7"
}; // endclass FTW_Nest

#endif
